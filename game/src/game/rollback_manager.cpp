#include <game/rollback_manager.h>
#include <game/game_manager.h>
#include "utils/assert.h"
#include <utils/log.h>
#include <fmt/format.h>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{

RollbackManager::RollbackManager(GameManager& gameManager, core::EntityManager& entityManager) :
    gameManager_(gameManager),entityManager_(entityManager),
    currentTransformManager_(entityManager),
    currentPhysicsManager_(entityManager),
	currentPlayerManager_(entityManager, currentPhysicsManager_, gameManager_),
    currentBulletManager_(entityManager, gameManager, currentPhysicsManager_),
    lastValidatePhysicsManager_(entityManager),
    lastValidatePlayerManager_(entityManager, lastValidatePhysicsManager_, gameManager_),
	lastValidateBulletManager_(entityManager, gameManager, lastValidatePhysicsManager_)
{
    for (auto& input : inputs_)
    {
        std::fill(input.begin(), input.end(), '\0');
    }
    currentPhysicsManager_.RegisterTriggerListener(*this);
}

void RollbackManager::SimulateToCurrentFrame()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto currentFrame = gameManager_.GetCurrentFrame();
    const auto lastValidateFrame = gameManager_.GetLastValidateFrame();
    //Destroying all created Entities after the last validated frame
    for (const auto& createdEntity : createdEntities_)
    {
        if (createdEntity.createdFrame > lastValidateFrame)
        {
            entityManager_.DestroyEntity(createdEntity.entity);
        }
    }
    createdEntities_.clear();
    //Remove DESTROY flags
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
        {
            entityManager_.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
        }
    }

    //Revert the current game state to the last validated game state
    currentBulletManager_.CopyAllComponents(lastValidateBulletManager_.GetAllComponents());
    currentPhysicsManager_.CopyAllComponents(lastValidatePhysicsManager_);
    currentPlayerManager_.CopyAllComponents(lastValidatePlayerManager_.GetAllComponents());

    for (Frame frame = lastValidateFrame + 1; frame <= currentFrame; frame++)
    {
        testedFrame_ = frame;
        //Copy player inputs to player manager
        for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
        {
            const auto playerInput = GetInputAtFrame(playerNumber, frame);
            const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
            if (playerEntity == core::INVALID_ENTITY)
            {
                core::LogWarning(fmt::format("Invalid Entity in {}:line {}", __FILE__, __LINE__));
                continue;
            }
            auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
            playerCharacter.input = playerInput;
            currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
        }
        //Simulate one frame of the game
        currentBulletManager_.FixedUpdate(sf::seconds(FIXED_PERIOD));
        currentPlayerManager_.FixedUpdate(sf::seconds(FIXED_PERIOD));
        currentPhysicsManager_.FixedUpdate(sf::seconds(FIXED_PERIOD));
    }
    //Copy the physics states to the transforms
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity,
            static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY) |
            static_cast<core::EntityMask>(core::ComponentType::TRANSFORM)))
            continue;
        const auto& body = currentPhysicsManager_.GetRigidbody(entity);
        currentTransformManager_.SetPosition(entity, body.position);
        currentTransformManager_.SetRotation(entity, body.rotation);
    }
}

void RollbackManager::SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, Frame inputFrame)
{
    //Should only be called on the server
    if (currentFrame_ < inputFrame)
    {
        StartNewFrame(inputFrame);
    }
    inputs_[playerNumber][currentFrame_ - inputFrame] = playerInput;
    if (lastReceivedFrame_[playerNumber] < inputFrame)
    {
        lastReceivedFrame_[playerNumber] = inputFrame;
        //Repeat the same inputs until currentFrame
        for (size_t i = 0; i < currentFrame_ - inputFrame; i++)
        {
            inputs_[playerNumber][i] = playerInput;
        }
    }
}

void RollbackManager::StartNewFrame(Frame newFrame)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (currentFrame_ > newFrame)
        return;
    const auto delta = newFrame - currentFrame_;
    if (delta == 0)
    {
        return;
    }
    for (auto& inputs : inputs_)
    {
        for (auto i = inputs.size() - 1; i >= delta; i--)
        {
            inputs[i] = inputs[i - delta];
        }

        for (Frame i = 0; i < delta; i++)
        {
            inputs[i] = inputs[delta];
        }
    }
    currentFrame_ = newFrame;
}

void RollbackManager::ValidateFrame(Frame newValidateFrame)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto lastValidateFrame = gameManager_.GetLastValidateFrame();
    //We check that we got all the inputs
    for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
    {
        if (GetLastReceivedFrame(playerNumber) < newValidateFrame)
        {
            gpr_assert(false, "We should not validate a frame if we did not receive all inputs!!!");
            return;
        }
    }
    //Destroying all created Entities after the last validated frame
    for (const auto& createdEntity : createdEntities_)
    {
        if (createdEntity.createdFrame > lastValidateFrame)
        {
            entityManager_.DestroyEntity(createdEntity.entity);
        }
    }
    createdEntities_.clear();
    //Remove DESTROYED flag
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
        {
            entityManager_.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
        }

    }
    createdEntities_.clear();

    //We use the current game state as the temporary new validate game state
    currentBulletManager_.CopyAllComponents(lastValidateBulletManager_.GetAllComponents());
    currentPhysicsManager_.CopyAllComponents(lastValidatePhysicsManager_);
    currentPlayerManager_.CopyAllComponents(lastValidatePlayerManager_.GetAllComponents());

    //We simulate the frames until the new validated frame
    for (Frame frame = lastValidateFrame_ + 1; frame <= newValidateFrame; frame++)
    {
        testedFrame_ = frame;
        //Copy the players inputs into the player manager
        for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
        {
            const auto playerInput = GetInputAtFrame(playerNumber, frame);
            const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
            auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
            playerCharacter.input = playerInput;
            currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
        }
        //We simulate one frame
        currentBulletManager_.FixedUpdate(sf::seconds(FIXED_PERIOD));
        currentPlayerManager_.FixedUpdate(sf::seconds(FIXED_PERIOD));
        currentPhysicsManager_.FixedUpdate(sf::seconds(FIXED_PERIOD));
    }
    //Definitely remove DESTROY entities
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
        {
            entityManager_.DestroyEntity(entity);
        }
    }
    //Copy back the new validate game state to the last validated game state
    lastValidateBulletManager_.CopyAllComponents(currentBulletManager_.GetAllComponents());
    lastValidatePlayerManager_.CopyAllComponents(currentPlayerManager_.GetAllComponents());
    lastValidatePhysicsManager_.CopyAllComponents(currentPhysicsManager_);
    lastValidateFrame_ = newValidateFrame;
    createdEntities_.clear();
}

void RollbackManager::ConfirmFrame(Frame newValidateFrame, const std::array<PhysicsState, MAX_PLAYER_NMB>& serverPhysicsState)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    ValidateFrame(newValidateFrame);
    for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
    {
        const PhysicsState lastPhysicsState = GetValidatePhysicsState(playerNumber);
        if (serverPhysicsState[playerNumber] != lastPhysicsState)
        {
            gpr_assert(false, fmt::format("Physics State are not equal for player {} (server frame: {}, client frame: {}, server: {}, client: {})", 
                playerNumber+1, 
                newValidateFrame, 
                lastValidateFrame_, 
                serverPhysicsState[playerNumber], 
                lastPhysicsState));
        }
    }
}

PhysicsState RollbackManager::GetValidatePhysicsState(PlayerNumber playerNumber) const
{
    PhysicsState state = 0;
    const core::Entity playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
    const auto& playerBody = lastValidatePhysicsManager_.GetRigidbody(playerEntity);

    const auto pos = playerBody.position;
    const auto* posPtr = reinterpret_cast<const PhysicsState*>(&pos);
    //Adding position
    for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
    {
        state += posPtr[i];
    }

    //Adding velocity
    const auto velocity = playerBody.velocity;
    const auto* velocityPtr = reinterpret_cast<const PhysicsState*>(&velocity);
    for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
    {
        state += velocityPtr[i];
    }
    //Adding rotation
    const auto angle = playerBody.rotation.value();
    const auto* anglePtr = reinterpret_cast<const PhysicsState*>(&angle);
    for (size_t i = 0; i < sizeof(float) / sizeof(PhysicsState); i++)
    {
        state += anglePtr[i];
    }
    //Adding angular Velocity
    const auto angularVelocity = playerBody.angularVelocity.value();
    const auto* angularVelPtr = reinterpret_cast<const PhysicsState*>(&angularVelocity);
    for (size_t i = 0; i < sizeof(float) / sizeof(PhysicsState); i++)
    {
        state += angularVelPtr[i];
    }
    return state;
}

void RollbackManager::SpawnPlayer(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::Vec2f lookDirection)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    Rigidbody playerBody;
    playerBody.position = position;
    //playerBody.bounciness = 0.0f;

    CircleCollider playerSphere;
    playerSphere.radius = 0.5f;

    PlayerCharacter playerCharacter;
    playerCharacter.playerNumber = playerNumber;
    playerCharacter.lookDir = lookDirection;
    playerCharacter.animationState = AnimationState::NONE;

    currentPlayerManager_.AddComponent(entity);
    currentPlayerManager_.SetComponent(entity, playerCharacter);

    currentPhysicsManager_.AddRigidbody(entity);
    currentPhysicsManager_.SetRigidbody(entity, playerBody);
    currentPhysicsManager_.AddCircle(entity);
    currentPhysicsManager_.SetCircle(entity, playerSphere);

    currentTransformManager_.AddComponent(entity);
	currentTransformManager_.SetPosition(entity, position);
    currentTransformManager_.SetRotation(entity, core::Degree{ 0.0f });
    currentTransformManager_.SetScale(entity, core::Vec2f{ PLAYER_SCALE.x  * lookDirection.x,PLAYER_SCALE.y});

    lastValidatePlayerManager_.AddComponent(entity);
    lastValidatePlayerManager_.SetComponent(entity, playerCharacter);

    lastValidatePhysicsManager_.AddRigidbody(entity);
    lastValidatePhysicsManager_.SetRigidbody(entity, playerBody);
    lastValidatePhysicsManager_.AddCircle(entity);
    lastValidatePhysicsManager_.SetCircle(entity, playerSphere);
}

PlayerInput RollbackManager::GetInputAtFrame(PlayerNumber playerNumber, Frame frame) const
{
    gpr_assert(currentFrame_ - frame < inputs_[playerNumber].size(),
        "Trying to get input too far in the past");
    return inputs_[playerNumber][currentFrame_ - frame];
}

void RollbackManager::OnTrigger(core::Entity entity1, core::Entity entity2)
{
    const std::function<void(core::Entity, core::Entity)> ManagePlayerCollision =
        [this](auto entity1, auto entity2)
    {
        auto player1Rigidbody = currentPhysicsManager_.GetRigidbody(entity1);
        auto player2Rigidbody = currentPhysicsManager_.GetRigidbody(entity2);
        auto mtv = currentPhysicsManager_.GetMTV();

        game::PhysicsManager::SolveCollision(player1Rigidbody, player2Rigidbody);
        game::PhysicsManager::SolveMTV(player1Rigidbody, player2Rigidbody, mtv);

        currentPhysicsManager_.SetRigidbody(entity1, player1Rigidbody);
        currentPhysicsManager_.SetRigidbody(entity2, player2Rigidbody);
    };

	const std::function<void(const PlayerCharacter&, core::Entity, const Bullet&, core::Entity)> ManagePlayerBulletCollision =
        [this](const auto& player, auto playerEntity, const auto& bullet, auto bulletEntity)
    {
        if (player.playerNumber != bullet.playerNumber)
        {
            auto playerRigidbody = currentPhysicsManager_.GetRigidbody(playerEntity);
            auto bulletRigidbody = currentPhysicsManager_.GetRigidbody(bulletEntity);
            gameManager_.DestroyBullet(bulletEntity);
            //lower health point
            auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
            if (playerCharacter.invincibilityTime <= 0.0f)
            {
                core::LogDebug(fmt::format("Player {} is hit by bullet", playerCharacter.playerNumber));
                //--playerCharacter.health;
                playerCharacter.invincibilityTime = PLAYER_INVINCIBILITY_PERIOD;
                playerRigidbody.velocity.x = bulletRigidbody.velocity.x;
            }
            currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
            currentPhysicsManager_.SetRigidbody(playerEntity, playerRigidbody);
        }
    };

    const std::function<void(core::Entity, Bullet, core::Entity, Bullet)> ManageBulletCollision =
        [this](auto entity1, auto bullet1, auto entity2, auto bullet2)
    {
        auto bullet1Rigidbody = currentPhysicsManager_.GetRigidbody(entity1);
        auto bullet2Rigidbody = currentPhysicsManager_.GetRigidbody(entity2);
        auto mtv = currentPhysicsManager_.GetMTV();

        game::PhysicsManager::SolveCollision(bullet1Rigidbody, bullet2Rigidbody);
        game::PhysicsManager::SolveMTV(bullet1Rigidbody, bullet2Rigidbody, mtv);

        currentPhysicsManager_.SetRigidbody(entity1, bullet1Rigidbody);
        currentPhysicsManager_.SetRigidbody(entity2, bullet2Rigidbody);

        gameManager_.DestroyBullet(entity1);
        gameManager_.DestroyBullet((entity2));
    };

    const std::function<void(core::Entity, core::Entity)> ManagePlatformCollision =
        [this](auto entity1, auto entity2)
    {
        auto playerRigidbody = currentPhysicsManager_.GetRigidbody(entity1);
        auto wallRigidbody = currentPhysicsManager_.GetRigidbody(entity2);
        auto mtv = currentPhysicsManager_.GetMTV();

        game::PhysicsManager::SolveCollision(playerRigidbody, wallRigidbody);
        game::PhysicsManager::SolveMTV(playerRigidbody, wallRigidbody, mtv);

        currentPhysicsManager_.SetRigidbody(entity1, playerRigidbody);
        currentPhysicsManager_.SetRigidbody(entity2, wallRigidbody);
    };

    //Collision between players
	if (entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)))
    {
        ManagePlayerCollision(entity1, entity2);
    }
    //Collision with player and bullet
    if (entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BULLET)))
    {
        const auto& player = currentPlayerManager_.GetComponent(entity1);
        const auto& bullet = currentBulletManager_.GetComponent(entity2);
        ManagePlayerBulletCollision(player, entity1, bullet, entity2);
    }
    if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BULLET)))
    {
        const auto& player = currentPlayerManager_.GetComponent(entity2);
        const auto& bullet = currentBulletManager_.GetComponent(entity1);
        ManagePlayerBulletCollision(player, entity2, bullet, entity1);
    }
    //Bullet collision
    if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BULLET)) &&
        entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BULLET)))
    {
        const auto& bullet1 = currentBulletManager_.GetComponent(entity1);
        const auto& bullet2 = currentBulletManager_.GetComponent(entity2);
        ManageBulletCollision(entity1, bullet1, entity2, bullet2);
    }
    //Player collides with platform
    if (entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BOX_COLLIDER)))
    {
        ManagePlatformCollision(entity1, entity2);
    }
    if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
        entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BOX_COLLIDER)))
    {
        ManagePlatformCollision(entity2, entity1);
    }
    
}

void RollbackManager::SpawnBullet(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::Vec2f velocity)
{
    createdEntities_.push_back({ entity, testedFrame_ });

    Rigidbody bulletBody;
    bulletBody.position = position;
    bulletBody.velocity = velocity;
    bulletBody.gravityScale = 0.0f;
    CircleCollider bulletSphere;
    bulletSphere.radius = 0.25f * BULLET_SCALE;

    currentBulletManager_.AddComponent(entity);
    currentBulletManager_.SetComponent(entity, { BULLET_PERIOD, playerNumber });

    currentPhysicsManager_.AddRigidbody(entity);
    currentPhysicsManager_.SetRigidbody(entity, bulletBody);
    currentPhysicsManager_.AddCircle(entity);
    currentPhysicsManager_.SetCircle(entity, bulletSphere);

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
    currentTransformManager_.SetScale(entity, core::Vec2f::one() * BULLET_SCALE);
    currentTransformManager_.SetRotation(entity, core::Degree(0.0f));
}

void RollbackManager::SpawnWall(core::Entity entity, core::Vec2f position)
{
    Rigidbody wallBody;
    wallBody.position = position;
    wallBody.bounciness = 1.0f;
    wallBody.bodyType = BodyType::STATIC;

    BoxCollider wallCollider;
    wallCollider.extends.x = WALL_SIZE.x;
	wallCollider.extends.y = WALL_SIZE.y;

    currentPhysicsManager_.AddRigidbody(entity);
    currentPhysicsManager_.SetRigidbody(entity, wallBody);
    currentPhysicsManager_.AddBox(entity);
    currentPhysicsManager_.SetBox(entity, wallCollider);

    lastValidatePhysicsManager_.AddRigidbody(entity);
    lastValidatePhysicsManager_.SetRigidbody(entity, wallBody);
    lastValidatePhysicsManager_.AddBox(entity);
    lastValidatePhysicsManager_.SetBox(entity, wallCollider);

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
    currentTransformManager_.SetRotation(entity, core::Degree{ 0.0f });
    currentTransformManager_.SetScale(entity, WALL_SIZE * WALL_SCALE);
}

void RollbackManager::DestroyEntity(core::Entity entity)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    //we don't need to save a bullet that has been created in the time window
    if (std::find_if(createdEntities_.begin(), createdEntities_.end(), [entity](auto newEntity)
        {
            return newEntity.entity == entity;
        }) != createdEntities_.end())
    {
        entityManager_.DestroyEntity(entity);
        return;
    }
        entityManager_.AddComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
}

//	void RollbackManager::RespawnPlayer(core::Entity entity)
//{
//    /*auto playerCharacter = currentPlayerManager_.GetComponent(entity);
//
//    core::LogDebug(fmt::format("Player {} fell of platform", playerCharacter.playerNumber));
//    --playerCharacter.health;
//
//    currentPlayerManager_.SetComponent(entity, playerCharacter);*/
//}
}

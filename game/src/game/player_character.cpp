#include <iostream>
#include <game/player_character.h>
#include <game/game_manager.h>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif
namespace game
{
PlayerCharacterManager::PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager) :
    ComponentManager(entityManager),
    physicsManager_(physicsManager),
    gameManager_(gameManager)

{

}

void PlayerCharacterManager::SetLookDirection(core::Entity entity, core::Vec2f lookDir)
{
    if (!entityManager_.HasComponent(entity,
        static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)))
    {
        return;
    }
    auto playerCharacter = GetComponent(entity);
    playerCharacter.lookDir = lookDir;
        
}

void PlayerCharacterManager::FixedUpdate(sf::Time dt)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
    {
        const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
        if (!entityManager_.HasComponent(playerEntity,
            static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)))
            continue;
        auto playerBody = physicsManager_.GetRigidbody(playerEntity);
        auto playerCharacter = GetComponent(playerEntity);
        const auto input = playerCharacter.input;

        const bool right = input & PlayerInputEnum::PlayerInput::RIGHT;
        const bool left = input & PlayerInputEnum::PlayerInput::LEFT;
        const bool up = input & PlayerInputEnum::PlayerInput::UP;
        //const bool down = input & PlayerInputEnum::PlayerInput::DOWN;

        //Set player movement
        const auto movement = ((left ? -1.0f : 0.0f) + (right ? 1.0f : 0.0f)) * PLAYER_SPEED;
        playerBody.velocity.x += movement * dt.asSeconds();
    	playerBody.velocity.x = (left ? playerBody.velocity.x : 0.0f) + (right ? playerBody.velocity.x : 0.0f);

		//Set player jump
    	const auto jump = (up ? 1.0f : 0.0f) * PLAYER_JUMP_FORCE;
        if(playerCharacter.isGrounded)
        {
            playerBody.velocity.y += jump;
            playerCharacter.isGrounded = false;
            SetComponent(playerEntity, playerCharacter);
        }
        if(playerBody.position.y <= -5.0f)
        {
            playerCharacter.isGrounded = true;
            SetComponent(playerEntity, playerCharacter);
        }

        //Set player's looking direction
        playerCharacter.lookDir =
            ((left ? core::Vec2f::left() : playerCharacter.lookDir) + (right ? core::Vec2f::right() : playerCharacter.lookDir));

    	physicsManager_.SetRigidbody(playerEntity, playerBody);

        if (playerCharacter.invincibilityTime > 0.0f)
        {
            playerCharacter.invincibilityTime -= dt.asSeconds();
            SetComponent(playerEntity, playerCharacter);
        }
        //Check if playerCharacter cannot shoot, and increase shootingTime
        if (playerCharacter.shootingTime < PLAYER_SHOOTING_PERIOD)
        {
            playerCharacter.shootingTime += dt.asSeconds();
            SetComponent(playerEntity, playerCharacter);
        }
        //Shooting mechanism
        if (playerCharacter.shootingTime >= PLAYER_SHOOTING_PERIOD)
        {
            if (input & PlayerInputEnum::PlayerInput::SHOOT)
            {
                const auto currentPlayerSpeed = playerBody.velocity.GetMagnitude();
                const auto bulletVelocity = playerCharacter.lookDir *
                    ((core::Vec2f::Dot(playerBody.velocity, playerCharacter.lookDir) > 0.0f ? currentPlayerSpeed : 0.0f)
                        + BULLET_SPEED);
                const auto bulletPosition = playerBody.position + playerCharacter.lookDir * 0.5f + playerBody.velocity * dt.asSeconds();
                gameManager_.SpawnBullet(playerCharacter.playerNumber,
                    bulletPosition,
                    bulletVelocity);
                playerCharacter.shootingTime = 0.0f;
                SetComponent(playerEntity, playerCharacter);
            }
        }
    }
}
}
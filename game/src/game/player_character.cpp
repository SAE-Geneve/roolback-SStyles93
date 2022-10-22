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
        const bool shoot = input & PlayerInputEnum::PlayerInput::SHOOT;

        //Set player movement
        const auto movement = ((left ? -1.0f : 0.0f) + (right ? 1.0f : 0.0f)) * PLAYER_SPEED;
        if(!playerCharacter.isShooting)
        {
            playerBody.velocity.x += movement * dt.asSeconds();
        }

    	//Set player jump
    	const auto jump = (up ? 1.0f : 0.0f) * PLAYER_JUMP_FORCE;
        if(playerCharacter.isGrounded)
        {
            playerBody.velocity.y = jump;
            playerCharacter.isGrounded = false;
        }

        //Set player AnimationState
        if(!playerCharacter.isShooting)
        {
            if (playerBody.position.y <= LOWER_LIMIT)
            {
                playerCharacter.isGrounded = true;
                playerCharacter.animationState = AnimationState::IDLE;

            }
            playerCharacter.animationState = (right || left) && !up && !shoot && playerCharacter.isGrounded ?
                AnimationState::WALK : playerCharacter.animationState;

            playerCharacter.animationState = up && !shoot ?
                AnimationState::JUMP : playerCharacter.animationState;

            ////UNCOMMENT / COMMENT to use shoot in the air or not
            playerCharacter.animationState = shoot ?
            //playerCharacter.animationState = shoot && playerCharacter.isGrounded ?
                AnimationState::SHOOT : playerCharacter.animationState;
        }
    	else
        {
            playerCharacter.animationState = AnimationState::SHOOT;
        }
        
    	//Set player's looking direction
        if(right)
        {
            playerCharacter.lookDir = core::Vec2f::right();
        }
        if (left)
        {
            playerCharacter.lookDir = core::Vec2f::left();
        }

    	SetComponent(playerEntity, playerCharacter);
        
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
            playerCharacter.isShooting = false;

            ////TO UNCOMMENT / COMMENT to use shoot in the air or not
            if (shoot)
            //if (shoot && playerCharacter.isGrounded)
            {
                const auto bulletVelocity = playerCharacter.lookDir * BULLET_SPEED;
                const auto bulletPosition = playerBody.position + playerCharacter.lookDir * 0.5f + playerBody.velocity * dt.asSeconds();
                gameManager_.SpawnBullet(playerCharacter.playerNumber,
                    bulletPosition,
                    bulletVelocity);

                playerCharacter.isShooting = true;
                playerCharacter.shootingTime = 0.0f;
            }

        	SetComponent(playerEntity, playerCharacter);

        }
    }
}
}

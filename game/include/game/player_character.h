#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"
#include "game/animation_manager.h"
#include "SFML/Graphics/Sprite.hpp"

namespace game
{
class PhysicsManager;
/**
 * \brief PlayerCharacter is a struct that holds information about the player character (when they can shoot again, their current input, and their current health).
 */
struct PlayerCharacter
{
    PlayerInput input = 0u;
    PlayerNumber playerNumber = INVALID_PLAYER;
    short health = PLAYER_HEALTH;
    float shootingTime = 0.0f;
    float invincibilityTime = 0.0f;
    bool isGrounded = false;
    bool isShooting = false;

    core::Vec2f lookDir = core::Vec2f::zero();
    AnimationState animationState = AnimationState::NONE;
};
class GameManager;

/**
 * \brief PlayerCharacterManager is a ComponentManager that holds all the PlayerCharacter in the game.
 */
class PlayerCharacterManager : public core::ComponentManager<PlayerCharacter, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)>
{
public:
    explicit PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager);
    void FixedUpdate(sf::Time dt);

private:
    PhysicsManager& physicsManager_;
    GameManager& gameManager_;
};
}

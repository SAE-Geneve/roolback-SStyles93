#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"

namespace game
{
/**
 * \brief Bullet is a struct that contains info about a player bullet (when it will be destroyed and whose player it is).
 */
struct Bullet
{
    PlayerNumber playerNumber = INVALID_PLAYER;
	float remainingTime = 0.0f;
    
    float power = 0.0f;
};

class GameManager;
class PhysicsManager;
class PlayerCharacterManager;

/**
 * \brief BulletManager is a ComponentManager that holds all the Bullet in one place.
 * It will automatically destroy the Bullet when remainingTime is over.
 */
class BulletManager : public core::ComponentManager<Bullet, static_cast<core::EntityMask>(ComponentType::BULLET)>
{
public:
    explicit BulletManager(
        core::EntityManager& entityManager, GameManager& gameManager, PhysicsManager& physicsManager);
    
    /**
     * @brief Updates the bullet manager
     * @param dt The delta time used for updating the manager
    */
    void FixedUpdate(sf::Time dt) const;

private:
    GameManager& gameManager_;
    PhysicsManager& physicsManager_;
};
}

#include "game/bullet_manager.h"
#include "game/game_manager.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif
namespace game
{
BulletManager::BulletManager(
    core::EntityManager& entityManager, GameManager& gameManager, PhysicsManager& physicsManager) :
    ComponentManager(entityManager),	gameManager_(gameManager), physicsManager_(physicsManager)
{
}

void BulletManager::FixedUpdate(sf::Time dt)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if(entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
        {
            continue;
        }
        if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::BULLET)))
        {
            auto bulletBody = physicsManager_.GetRigidbody(entity);

            if(bulletBody.velocity.x > 0.0f)
            {
                bulletBody.rotation += dt.asSeconds() * BULLET_ROTATION_SPEED;
            }
            else
            {
                bulletBody.rotation -= dt.asSeconds() * BULLET_ROTATION_SPEED;
            }
            physicsManager_.SetRigidbody(entity, bulletBody);

            auto& bullet = components_[entity];
            /*bullet.remainingTime -= dt.asSeconds();*/
            if (/*bullet.remainingTime < 0.0f ||*/ 
                bulletBody.position.x <= LEFT_LIMIT * 0.95f || 
                bulletBody.position.x >= RIGHT_LIMIT * 0.95f)
            {
                gameManager_.DestroyBullet(entity);
            }
        }
    }
}
}

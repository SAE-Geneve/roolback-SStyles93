#include "game/physics_manager.h"
#include "engine/transform.h"

#include <SFML/Graphics/CircleShape.hpp>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{

PhysicsManager::PhysicsManager(core::EntityManager& entityManager) :
    entityManager_(entityManager), rigidbodyManager_(entityManager), sphereColliderManager_(entityManager)
{

}

bool IsOverlappingSphere(
    SphereCollider mySphere, Rigidbody myBody, 
    SphereCollider otherSphere, Rigidbody otherBody, 
    core::Vec2f& mtv)
{
    const  core::Vec2f distance = otherBody.position - myBody.position;

    const float distanceMagnitude = distance.GetMagnitude();
	const float radiusSum = mySphere.radius + otherSphere.radius;

    const float mtvDifference = radiusSum - distanceMagnitude;
    mtv = distance.GetNormalized() * mtvDifference;

    return (distanceMagnitude <= radiusSum);
}

//constexpr bool Box2Box(float r1x, float r1y, float r1w, float r1h, float r2x, float r2y, float r2w, float r2h)
//{
//    return r1x + r1w >= r2x &&    // r1 right edge past r2 left
//        r1x <= r2x + r2w &&    // r1 left edge past r2 right
//        r1y + r1h >= r2y &&    // r1 top edge past r2 bottom
//        r1y <= r2y + r2h;
//}

void PhysicsManager::FixedUpdate(sf::Time dt)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity, static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY)))
            continue;
        auto rigidbody = rigidbodyManager_.GetComponent(entity);
        rigidbody.position += rigidbody.velocity * dt.asSeconds();
        rigidbody.rotation += rigidbody.angularVelocity * dt.asSeconds();
        rigidbodyManager_.SetComponent(entity, rigidbody);
    }
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity,
            static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY) |
            static_cast<core::EntityMask>(core::ComponentType::SPHERECOLLIDER)) ||
            entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
            continue;
        for (core::Entity otherEntity = entity + 1; otherEntity < entityManager_.GetEntitiesSize(); otherEntity++)
        {
            if (!entityManager_.HasComponent(otherEntity,
                static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY) | static_cast<core::EntityMask>(core::ComponentType::SPHERECOLLIDER)) ||
                entityManager_.HasComponent(otherEntity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
                continue;
            const Rigidbody& rigidbody1 = rigidbodyManager_.GetComponent(entity);
            const SphereCollider& sphere1 = sphereColliderManager_.GetComponent(entity);

            const Rigidbody& rigidbody2 = rigidbodyManager_.GetComponent(otherEntity);
            const SphereCollider& sphere2 = sphereColliderManager_.GetComponent(otherEntity);
            
            if(IsOverlappingSphere(sphere1, rigidbody1, sphere2, rigidbody2, mtv_))
            {
                onTriggerAction_.Execute(entity, otherEntity);
            }

        }
    }
}

void PhysicsManager::AddRigidbody(core::Entity entity)
{
    rigidbodyManager_.AddComponent(entity);
}

void PhysicsManager::SetRigidbody(core::Entity entity, const Rigidbody& rigidbody)
{
    rigidbodyManager_.SetComponent(entity, rigidbody);
}

const Rigidbody& PhysicsManager::GetRigidbody(core::Entity entity) const
{
    return rigidbodyManager_.GetComponent(entity);
}

void PhysicsManager::AddSphere(core::Entity entity)
{
    sphereColliderManager_.AddComponent(entity);
}

void PhysicsManager::SetSphere(core::Entity entity, const SphereCollider& sphere)
{
    sphereColliderManager_.SetComponent(entity, sphere);
}

const SphereCollider& PhysicsManager::GetSphere(core::Entity entity) const
{
    return sphereColliderManager_.GetComponent(entity);
}

void PhysicsManager::RegisterTriggerListener(OnTriggerInterface& onTriggerInterface)
{
    onTriggerAction_.RegisterCallback(
        [&onTriggerInterface](core::Entity entity1, core::Entity entity2) { onTriggerInterface.OnTrigger(entity1, entity2); });
}

void PhysicsManager::CopyAllComponents(const PhysicsManager& physicsManager)
{
    rigidbodyManager_.CopyAllComponents(physicsManager.rigidbodyManager_.GetAllComponents());
    sphereColliderManager_.CopyAllComponents(physicsManager.sphereColliderManager_.GetAllComponents());
}

void PhysicsManager::Draw(sf::RenderTarget& renderTarget)
{
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity,
            static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY) |
            static_cast<core::EntityMask>(core::ComponentType::SPHERECOLLIDER)) ||
            entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
            continue;
        const auto& [radius, isTrigger] = sphereColliderManager_.GetComponent(entity);
        const auto& body = rigidbodyManager_.GetComponent(entity);
        sf::CircleShape circleShape;
        circleShape.setFillColor(core::Color::transparent());
        //circleShape.setFillColor(core::Color::green());
        circleShape.setOutlineColor(core::Color::green());
        circleShape.setOutlineThickness(2.0f);
        const auto position = body.position;
        circleShape.setOrigin(radius * core::PIXEL_PER_METER, radius * core::PIXEL_PER_METER);
        circleShape.setPosition(
            position.x * core::PIXEL_PER_METER + center_.x,
            windowSize_.y - (position.y * core::PIXEL_PER_METER + center_.y));
        circleShape.setRadius(radius * core::PIXEL_PER_METER);
        renderTarget.draw(circleShape);
    }
}
}

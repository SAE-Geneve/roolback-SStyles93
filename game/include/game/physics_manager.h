#pragma once
#include "game_globals.h"
#include "engine/component.h"
#include "engine/entity.h"
#include "maths/angle.h"
#include "maths/vec2.h"

#include <SFML/System/Time.hpp>

#include "graphics/graphics.h"
#include "utils/action_utility.h"

namespace core
{
class TransformManager;
}

namespace game
{
enum class BodyType
{
    DYNAMIC,
    STATIC
};

/**
 * \brief SphereCollider is a spherical shape collider used in the physics engine
 */
struct SphereCollider
{
    float radius = 0.5f;
    bool isTrigger = false;
};
/**
 * \brief Rigidbody is a class that represents a rigid body.
 */
struct Rigidbody
{
    core::Vec2f position = core::Vec2f::zero();
    core::Degree rotation = core::Degree(0.0f);

    core::Vec2f velocity = core::Vec2f::zero();
    core::Degree angularVelocity = core::Degree(0.0f);

	core::Vec2f acceleration = core::Vec2f::zero();
    core::Vec2f force = core::Vec2f::zero();

    float bounciness = 1.0f;
    float gravityScale = 1.0f;
    float mass = 0.0f;

    BodyType bodyType = BodyType::DYNAMIC;
};

/**
 * \brief OnTriggerInterface is an interface for classes that needs to be called when two boxes are in contact.
 * It needs to be registered in the PhysicsManager.
 */
class OnTriggerInterface
{
public:
    virtual ~OnTriggerInterface() = default;
    virtual void OnTrigger(core::Entity entity1, core::Entity entity2) = 0;
};

/**
 * \brief RigidbodyManager is a ComponentManager that holds all the Rigidbodies in the world.
 */
class RigidbodyManager : public core::ComponentManager<Rigidbody, static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY)>
{
public:
    using ComponentManager::ComponentManager;
};

/**
 * \brief BoxManager is a ComponentManager that holds all the Box in the world.
 */
class SphereColliderManager : public core::ComponentManager<SphereCollider, static_cast<core::EntityMask>(core::ComponentType::SPHERECOLLIDER)>
{
public:
    using ComponentManager::ComponentManager;
};

/**
 * \brief PhysicsManager is a class that holds both BodyManager and BoxManager and manages the physics fixed update.
 * It allows to register OnTriggerInterface to be called when a trigger occcurs.
 */
class PhysicsManager : public core::DrawInterface
{
public:
    explicit PhysicsManager(core::EntityManager& entityManager);
    void FixedUpdate(sf::Time dt);

    void SetRigidbody(core::Entity entity, const Rigidbody& rigidbody);
    void AddRigidbody(core::Entity entity);
    [[nodiscard]] const Rigidbody& GetRigidbody(core::Entity entity) const;

    void AddSphere(core::Entity entity);
    void SetSphere(core::Entity entity, const SphereCollider& sphere);
    [[nodiscard]] const SphereCollider& GetSphere(core::Entity entity) const;
    /**
     * \brief RegisterTriggerListener is a method that stores an OnTriggerInterface in the PhysicsManager that will call the OnTrigger method in case of a trigger.
     * \param onTriggerInterface is the OnTriggerInterface to be called when a trigger occurs.
     */
    void RegisterTriggerListener(OnTriggerInterface& onTriggerInterface);
    void CopyAllComponents(const PhysicsManager& physicsManager);
    void Draw(sf::RenderTarget& renderTarget) override;
    void SetCenter(sf::Vector2f center) { center_ = center; }
    void SetWindowSize(sf::Vector2f newWindowSize) { windowSize_ = newWindowSize; }

    static void SolveCollision(Rigidbody& myBody, Rigidbody& otherBody);
    static void SolveMTV(Rigidbody& myBody, Rigidbody& otherBody, const core::Vec2f& mtv);

    [[nodiscard]] core::Vec2f GetMTV() const { return mtv_; }

private:

    core::EntityManager& entityManager_;
    RigidbodyManager rigidbodyManager_;
    SphereColliderManager sphereColliderManager_;
    core::Action<core::Entity, core::Entity> onTriggerAction_;
    //Used for debug
    sf::Vector2f center_{};
    sf::Vector2f windowSize_{};

    core::Vec2f mtv_{};

};

}

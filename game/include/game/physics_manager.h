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
 * \brief CircleCollider is a circle shape collider used in the physics engine
 */
struct CircleCollider
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

    BodyType bodyType = BodyType::DYNAMIC;

    float bounciness = 1.0f;
    float gravityScale = 1.0f;
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
class CircleColliderManager : public core::ComponentManager<CircleCollider, static_cast<core::EntityMask>(core::ComponentType::CIRCLE_COLLIDER)>
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

    /**
     * @brief Applies gravtity to all Rigidbodies
     * @param dt The delta time used to update
    */
    void ApplyGravityToRigidbodies(sf::Time dt);
    /**
     * @brief Method to limit le player's movement to a defined box
     * @param dt The delta time used to update
    */
    void LimitPlayerMovement(sf::Time dt);
    /**
     * @brief Checks for collisions between circle colliders
    */
    void CheckForCircleCollisions();
    /**
     * @brief The physical update
     * @param dt The time used to update
    */
    void FixedUpdate(sf::Time dt);

    /**
     * @brief Adds a rigidbody to an entity
     * @param entity the entity to which we add a rigidbody
    */
    void AddRigidbody(core::Entity entity);
    /**
     * @brief Sets an Entity's rigidboy to the given rigidbody
     * @param entity The entity to set
     * @param rigidbody The given rigidbody
    */
    void SetRigidbody(core::Entity entity, const Rigidbody& rigidbody);
    [[nodiscard]] const Rigidbody& GetRigidbody(core::Entity entity) const;

    /**
     * @brief Add a circle collider to and entity
     * @param entity The entity to which we add a cirlce collider
    */
    void AddCircle(core::Entity entity);
    /**
     * @brief Sets an Entity's circle collider to the given rigidbody
     * @param entity The entity to set
     * @param
     */
    void SetCircle(core::Entity entity, const CircleCollider& sphere);
    [[nodiscard]] const CircleCollider& GetCircle(core::Entity entity) const;

    /**
     * \brief RegisterTriggerListener is a method that stores an OnTriggerInterface in the PhysicsManager that will call the OnTrigger method in case of a trigger.
     * \param onTriggerInterface is the OnTriggerInterface to be called when a trigger occurs.
     */
    void RegisterTriggerListener(OnTriggerInterface& onTriggerInterface);
    /**
     * @brief Copies all the element of the physics manager
     * @param physicsManager The physics manager to copy from
    */
    void CopyAllComponents(const PhysicsManager& physicsManager);
    /**
     * @brief Draws the shapes of the physical elements
     * @param renderTarget the target to render the shapes on
    */
    void Draw(sf::RenderTarget& renderTarget) override;
    void SetCenter(sf::Vector2f center) { center_ = center; }
    void SetWindowSize(sf::Vector2f newWindowSize) { windowSize_ = newWindowSize; }

    /**
     * @brief Solves the collisions between two rigidbodies
     * @param myBody The first rigidbody to evaluate
     * @param otherBody The second rigidbody to evaluate
    */
    static void SolveCollision(Rigidbody& myBody, Rigidbody& otherBody);
    /**
     * @brief Solves the position of two rigidbodies
     * @param myBody The first rigidbody to evaluate
     * @param otherBody The second rigidbody to evaluate
     * @param mtv The minimum translation vector used to set rigidbodies positions
    */
    static void SolveMTV(Rigidbody& myBody, Rigidbody& otherBody, const core::Vec2f& mtv);

    [[nodiscard]] core::Vec2f GetMTV() const { return mtv_; }

private:

    core::EntityManager& entityManager_;
    RigidbodyManager rigidbodyManager_;
    CircleColliderManager circleColliderManager_;
    core::Action<core::Entity, core::Entity> onTriggerAction_;
    //Used for debug
    sf::Vector2f center_{};
    sf::Vector2f windowSize_{};

    core::Vec2f mtv_{};

};

}

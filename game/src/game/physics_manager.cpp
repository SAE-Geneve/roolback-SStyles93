#include "game/physics_manager.h"

#include "engine/transform.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{

PhysicsManager::PhysicsManager(core::EntityManager& entityManager) :
	entityManager_(entityManager), rigidbodyManager_(entityManager),
	circleColliderManager_(entityManager){}

/**
 * \brief Checks for overlapping between two spheres
 * \param myCircle The first sphere to evaluate
 * \param myBody The first rigidbody to evaluate
 * \param otherCircle The second  sphere to evaluate
 * \param otherBody The second rigidbody to evaluate
 * \param mtv The minimum translation vector used in collision solving
 * \return True if two spheres are overlapping
 */
bool IsOverlappingCircle(
	CircleCollider myCircle, Rigidbody myBody,
	CircleCollider otherCircle, Rigidbody otherBody,
	core::Vec2f& mtv)
{
	const  core::Vec2f distance = otherBody.position - myBody.position;

	const float distanceMagnitude = distance.GetMagnitude();
	const float radiusSum = myCircle.radius + otherCircle.radius;

	const float mtvDifference = radiusSum - distanceMagnitude;
	mtv = distance.GetNormalized() * mtvDifference;

	return (distanceMagnitude <= radiusSum);
}

/**
 * \brief Solves collisions between two rigidbodies
 * \param myBody The first rigidbody to evaluate and modify
 * \param otherBody The second rigidbody to evaluate and modify
 */
void PhysicsManager::SolveCollision(Rigidbody& myBody, Rigidbody& otherBody)
{
	const core::Vec2f v1 = myBody.velocity;
	const core::Vec2f v2 = otherBody.velocity;

	core::Vec2f n = core::Vec2f(otherBody.position - myBody.position).GetNormalized();
	core::Vec2f g = n.RightOrtho();

	const float v1n = core::Vec2f::Dot(n, v1);
	const float v1g = core::Vec2f::Dot(g, v1);
	const float v2n = core::Vec2f::Dot(n, v2);
	const float v2g = core::Vec2f::Dot(g, v2);

	const core::Vec2f v1AfterImpact = core::Vec2f(n.x * v2n + g.x * v1g, n.y * v2n + g.y * v1g);
	const core::Vec2f v2AfterImpact = core::Vec2f(n.x * v1n + g.x * v2g, n.y * v1n + g.y * v2g);

	if (!myBody.hasCollided || !otherBody.hasCollided) 
	{
		if (myBody.bodyType == BodyType::DYNAMIC)
		{
			myBody.velocity = v1AfterImpact * myBody.bounciness;
		}
		else
		{
			otherBody.velocity = v1AfterImpact + (v2AfterImpact * -1.0f) * otherBody.bounciness;
		}
		myBody.hasCollided = true;
	
		if (otherBody.bodyType == BodyType::DYNAMIC)
		{
			otherBody.velocity = v2AfterImpact * otherBody.bounciness;
		}
		else
		{
			myBody.velocity = v1AfterImpact + (v2AfterImpact * -1.0f) * myBody.bounciness;
		}
		otherBody.hasCollided = true;
	}
}
/**
 * \brief Solves the new positions of given rigidbodies
 * \param myBody The first body to be modified
 * \param otherBody The second body to be modified
 * \param mtv The minimum translation vector given to solve the new positions of rigidbodies
 */
void PhysicsManager::SolveMTV(Rigidbody& myBody, Rigidbody& otherBody, const core::Vec2f& mtv)
{
	if (mtv.GetSqrMagnitude() > 0.0f)
	{
		if (myBody.bodyType == BodyType::DYNAMIC)
			myBody.position = myBody.position - (mtv * 0.5f);
		if (otherBody.bodyType == BodyType::DYNAMIC)
			otherBody.position = otherBody.position + (mtv * 0.5f);
	}
}


void PhysicsManager::ApplyGravityToRigidbodies(sf::Time dt)
{
	//General entities
	for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
	{
		if (!entityManager_.HasComponent(entity, static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY)))
			continue;
		auto rigidbody = rigidbodyManager_.GetComponent(entity);

		//Apply gravity
		if (rigidbody.position.y > LOWER_LIMIT && rigidbody.bodyType == BodyType::DYNAMIC)
		{
			rigidbody.velocity.y += (GRAVITY * rigidbody.gravityScale) * dt.asSeconds();
		}

		rigidbody.position += rigidbody.velocity * dt.asSeconds();

		rigidbodyManager_.SetComponent(entity, rigidbody);
	}
}

void PhysicsManager::LimitPlayerMovement(sf::Time dt)
{
	for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
	{
		//Check for sphere collisions
		if (!entityManager_.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY) |
			static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) ||
			entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
			continue;

		auto rigidbody = rigidbodyManager_.GetComponent(entity);

		if (rigidbody.position.y < LOWER_LIMIT)
		{
			rigidbody.position.y = LOWER_LIMIT;
			//Kill player
		}
		//Block positions in limits
		if (rigidbody.position.y > UPPER_LIMIT)
		{
			rigidbody.position.y = UPPER_LIMIT;
		}
		if (rigidbody.position.x > RIGHT_LIMIT)
		{
			rigidbody.position.x = RIGHT_LIMIT;
		}
		if (rigidbody.position.x < LEFT_LIMIT)
		{
			rigidbody.position.x = LEFT_LIMIT;
		}

		//Reduce velocity over time
		if (rigidbody.velocity.x > 0.0f || rigidbody.velocity.x < 0.0f)
		{
			rigidbody.velocity.x += (0.0f - rigidbody.velocity.x) * (dt.asSeconds() * 2.0f);
		}

		rigidbodyManager_.SetComponent(entity, rigidbody);
	}
}

void PhysicsManager::CheckForCircleCollisions()
{
	for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
	{
		//Check for sphere collisions
		if (!entityManager_.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY) |
			static_cast<core::EntityMask>(core::ComponentType::CIRCLE_COLLIDER)) ||
			entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
			continue;
		

		for (core::Entity otherEntity = entity + 1; otherEntity < entityManager_.GetEntitiesSize(); otherEntity++)
		{
			if (!entityManager_.HasComponent(otherEntity,
				static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY) | static_cast<core::EntityMask>(core::ComponentType::CIRCLE_COLLIDER)) ||
				entityManager_.HasComponent(otherEntity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
				continue;

			if (!entityManager_.EntityExists(entity) || !entityManager_.EntityExists(otherEntity))
				continue;

			const Rigidbody& rigidbody1 = rigidbodyManager_.GetComponent(entity);
			const CircleCollider& circle1 = circleColliderManager_.GetComponent(entity);

			const Rigidbody& rigidbody2 = rigidbodyManager_.GetComponent(otherEntity);
			const CircleCollider& circle2 = circleColliderManager_.GetComponent(otherEntity);

			if (IsOverlappingCircle(circle1, rigidbody1, circle2, rigidbody2, mtv_))
			{
				onTriggerAction_.Execute(entity, otherEntity);
			}
		}
	}
}


void PhysicsManager::FixedUpdate(sf::Time dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	ApplyGravityToRigidbodies(dt);
	LimitPlayerMovement(dt);
	CheckForCircleCollisions();
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

void PhysicsManager::AddCircle(core::Entity entity)
{
	circleColliderManager_.AddComponent(entity);
}

void PhysicsManager::SetCircle(core::Entity entity, const CircleCollider& sphere)
{
	circleColliderManager_.SetComponent(entity, sphere);
}

const CircleCollider& PhysicsManager::GetCircle(core::Entity entity) const
{
	return circleColliderManager_.GetComponent(entity);
}

void PhysicsManager::RegisterTriggerListener(OnTriggerInterface& onTriggerInterface)
{
	onTriggerAction_.RegisterCallback(
		[&onTriggerInterface](core::Entity entity1, core::Entity entity2) { onTriggerInterface.OnTrigger(entity1, entity2); });
}

void PhysicsManager::CopyAllComponents(const PhysicsManager& physicsManager)
{
	rigidbodyManager_.CopyAllComponents(physicsManager.rigidbodyManager_.GetAllComponents());
	circleColliderManager_.CopyAllComponents(physicsManager.circleColliderManager_.GetAllComponents());
}

void PhysicsManager::Draw(sf::RenderTarget& renderTarget)
{
	for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
	{
		if (!entityManager_.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::RIGIDBODY) |
			static_cast<core::EntityMask>(core::ComponentType::CIRCLE_COLLIDER)) ||
			entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
			continue;
		const auto& [radius, isTrigger] = circleColliderManager_.GetComponent(entity);
		const auto& sphereBody = rigidbodyManager_.GetComponent(entity);
		sf::CircleShape circleShape;
		circleShape.setFillColor(core::Color::transparent());
		//circleShape.setFillColor(core::Color::green());
		circleShape.setOutlineColor(core::Color::green());
		circleShape.setOutlineThickness(2.0f);
		const auto spherePosition = sphereBody.position;
		circleShape.setOrigin(radius * core::PIXEL_PER_METER, radius * core::PIXEL_PER_METER);
		circleShape.setPosition(
			spherePosition.x * core::PIXEL_PER_METER + center_.x,
			windowSize_.y - (spherePosition.y * core::PIXEL_PER_METER + center_.y));
		circleShape.setRadius(radius * core::PIXEL_PER_METER);
		renderTarget.draw(circleShape);
	}
}
}

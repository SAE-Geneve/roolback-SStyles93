#include "engine/entity.h"
#include "engine/component.h"
#include "utils/assert.h"

#include <algorithm>

namespace core
{
EntityManager::EntityManager()
{
    entityMasks_.resize(ENTITY_INIT_NMB, INVALID_ENTITY_MASK);
}

EntityManager::EntityManager(std::size_t reservedSize)
{
    entityMasks_.resize(reservedSize, INVALID_ENTITY_MASK);
}

Entity EntityManager::CreateEntity()
{
    const auto entityMaskIt = std::find_if(entityMasks_.begin(), entityMasks_.end(),
        [](EntityMask entityMask)
        {
            return entityMask == INVALID_ENTITY_MASK;
        });

    if (entityMaskIt == entityMasks_.end())
    {
        const auto newEntity = entityMasks_.size();
        entityMasks_.resize(newEntity + newEntity / 2, INVALID_ENTITY_MASK);
        AddComponent(
            static_cast<Entity>(newEntity),
            static_cast<EntityMask>(ComponentType::EMPTY));
        return static_cast<Entity>(newEntity);
    }
    
    const auto newEntity = std::distance(entityMasks_.begin(), entityMaskIt);
    AddComponent(
        static_cast<Entity>(newEntity), 
        static_cast<EntityMask>(ComponentType::EMPTY));
    return static_cast<Entity>(newEntity);
}

void EntityManager::DestroyEntity(Entity entity)
{
    gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
    entityMasks_[entity] = INVALID_ENTITY_MASK;
}

void EntityManager::AddComponent(Entity entity, EntityMask mask)
{
    gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
    entityMasks_[entity] |= mask;
}

void EntityManager::RemoveComponent(Entity entity, EntityMask mask)
{
    gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
    entityMasks_[entity] &= ~mask;

}

bool EntityManager::EntityExists(Entity entity) const
{
    gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
    return entityMasks_[entity] != INVALID_ENTITY_MASK;
}

std::size_t EntityManager::GetEntitiesSize() const
{
    return entityMasks_.size();
}

bool EntityManager::HasComponent(Entity entity, EntityMask mask) const
{
    gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
    return (entityMasks_[entity] & mask) == mask;
}
}

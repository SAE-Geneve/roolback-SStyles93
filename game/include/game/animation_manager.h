#pragma once
#include <filesystem>
#include <SFML/System/Time.hpp>

#include "game_globals.h"
#include "SFML/Graphics/Texture.hpp"


namespace core
{
	class SpriteManager;
}

namespace game
{
class GameManager;

enum class AnimationState
{
    IDLE,
    WALK,
    JUMP,
    SHOOT,
    NONE
};
struct AnimationData
{
    float time = 0;
    int textureIdx = 0;
    AnimationState animationState = AnimationState::NONE;
};
struct Animation
{
    std::vector<sf::Texture> textures{};
};
/**
 * \brief AnimationManager is a ComponentManager that holds all the animations in one place.
 */
class AnimationManager : public core::ComponentManager<AnimationData, static_cast<core::EntityMask>(ComponentType::ANIMATION)>
{

public:

    AnimationManager(core::EntityManager& entityManager, core::SpriteManager& spriteManager, GameManager& gameManager);
   
    /**
     * @brief Loads the given texture form a path in the given animation
     * @param path The animation name
     * @param animation The animation to load it in
    */
    void LoadTexture(std::string_view path, Animation& animation) const;
    
    /**
     * @brief Plays an animation once
     * @param entity The entity for which we want to play an animation
     * @param animation The animation to play
     * @param animationData The animation data to "drive" the animation
     * @param speed The speed at which we want the animation to play
    */
    void PlayAnimation(const core::Entity& entity, const Animation& animation, AnimationData& animationData, float speed) const;
    /**
     * @brief Plays an animation multiple times (loop)
     * @param entity The entity for which we want to play an animation
     * @param animation The animation to play
     * @param animationData The animation data to "drive" the animation
     * @param speed The speed at which we want the animation to play
    */
    void LoopAnimation(const core::Entity& entity, const Animation& animation, AnimationData& animationData, float speed) const;

    /**
     * @brief Updates the animation on an entity
     * @param entity The entity to update
     * @param animationState The animation state of the entity
     * @param dt The delta time
    */
    void UpdateEntity(core::Entity entity, AnimationState animationState, sf::Time dt);

    Animation catIdle;
    Animation catWalk;
	Animation catJump;
	Animation catShoot;

private:

	core::SpriteManager& spriteManager_;
    GameManager& gameManager_;
};
}


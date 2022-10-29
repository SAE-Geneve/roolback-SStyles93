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
   
    void LoadTexture(std::string_view path, Animation& animation) const;
    
    void PlayAnimation(const core::Entity& entity, const Animation& animation, AnimationData& animationData, float speed) const;
    void LoopAnimation(const core::Entity& entity, const Animation& animation, AnimationData& animationData, float speed) const;

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


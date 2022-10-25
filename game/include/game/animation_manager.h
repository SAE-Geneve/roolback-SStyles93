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
struct Animation
{
    int textureIdx = 0;
    std::vector<sf::Texture> textures{};
};
/**
 * \brief AnimationManager is a ComponentManager that holds all the animations in one place.
 */
class AnimationManager : public core::ComponentManager<Animation, static_cast<core::EntityMask>(ComponentType::ANIMATION)>
{

public:

    AnimationManager(core::EntityManager& entityManager, core::SpriteManager& spriteManager, GameManager& gameManager);

   
    void LoadTexture(std::string_view path, Animation& animation) const;
    
    void PlayAnimation(const core::Entity& entity, Animation& animation, float speed);

    void UpdateEntity(core::Entity entity, AnimationState animationState, sf::Time dt);

    float animationTime_ = 0;

    Animation catIdle;
    Animation catWalk;
	Animation catJump;
	Animation catShoot;

private:

	core::SpriteManager& spriteManager_;
    GameManager& gameManager_;
};
}


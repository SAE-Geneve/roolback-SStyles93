#include "game/animation_manager.h"
#include "game/game_manager.h"
#include <fmt/format.h>

namespace game
{
AnimationManager::AnimationManager(core::EntityManager& entityManager, core::SpriteManager& spriteManager, GameManager& gameManager) :
	ComponentManager(entityManager), spriteManager_(spriteManager), gameManager_(gameManager)
{}

void AnimationManager::LoadTexture(const std::string_view path, Animation& animation) const
{
	auto format = fmt::format("data/sprites/{}", path);
	auto dirIter = std::filesystem::directory_iterator(fmt::format("data/sprites/{}",path));
	const int textureCount = std::count_if(
		begin(dirIter),
		end(dirIter),
		[](auto& entry) { return entry.is_regular_file() && entry.path().extension() == ".png"; });

	//LOAD SPRITES
	for (size_t i = 0; i < textureCount; i++)
	{
		sf::Texture newTexture;
		const auto fullPath = fmt::format("data/sprites/{}/{}{}.png",path, path, i);
		if (!newTexture.loadFromFile(fullPath))
		{
			core::LogError(fmt::format("Could not load data/sprites/{}/{}{}.png sprite",path, path, i));
		}
		animation.textures.push_back(newTexture);
	}
}

void AnimationManager::PlayAnimation(const core::Entity& entity,Animation& animation, float speed)
{
	auto& playerCharacter = gameManager_.GetRollbackManager().GetPlayerCharacterManager().GetComponent(entity);
	auto& playerSprite = spriteManager_.GetComponent(entity);
	
	
	switch (playerCharacter.animationState)
	{
	case AnimationState::IDLE:
	case AnimationState::WALK:
		//Ignores animations if the "cat_shoot" animation is playing
		if(playerCharacter.isShooting) 
			break;
		//Plays animation
		if (animationTime_ >= ANIMATION_PERIOD / speed)
		{
			animation.textureIdx++;
			if (animation.textureIdx >= animation.textures.size())
			{
				//resets the animation (it loops)
				animation.textureIdx = 0;
			}
			animationTime_ = 0;
		}
		//Prevents texture index from being out of range of the textures vector
		if (animation.textureIdx >= animation.textures.size())
		{
			animation.textureIdx = animation.textures.size() - 1;
		}
		playerSprite.setTexture(animation.textures[animation.textureIdx]);
		break;

	case AnimationState::JUMP:
		if (playerCharacter.isShooting)
			break;

		if (playerCharacter.isGrounded)
		{
			animation.textureIdx = 0;
		}
		if (animationTime_ >= ANIMATION_PERIOD / speed)
		{
			animation.textureIdx++;
			if (animation.textureIdx >= animation.textures.size())
			{
				//blocks the animation on last texture
				animation.textureIdx = animation.textures.size() - 1;
			}
			animationTime_ = 0;
		}
		if (animation.textureIdx >= animation.textures.size())
		{
			animation.textureIdx = animation.textures.size() - 1;
		}
		playerSprite.setTexture(animation.textures[animation.textureIdx]);
		break;

	case AnimationState::SHOOT:
		if (animationTime_ >= ANIMATION_PERIOD / speed)
		{
			animation.textureIdx++;
			if (animation.textureIdx >= animation.textures.size())
			{
					animation.textureIdx = 0;
			}
			animationTime_ = 0;
		}
		if (animation.textureIdx >= animation.textures.size())
		{
			animation.textureIdx = animation.textures.size() - 1;
		}
		playerSprite.setTexture(animation.textures[animation.textureIdx]);
		break;

	case AnimationState::NONE:
		break;

	default:
		core::LogError("PlayAnimation not working as expected");
		break;
	}
}

void AnimationManager::UpdateEntity(core::Entity entity, AnimationState animationState, sf::Time dt)
{
	animationTime_ += dt.asSeconds();
	switch (animationState)
	{
	case AnimationState::IDLE:
		PlayAnimation(entity, catIdle, 1.0f);
		break;

	case AnimationState::WALK:
		PlayAnimation(entity, catWalk, 1.0f);
		break;

	case AnimationState::JUMP:
		PlayAnimation(entity, catJump, 2.0f);
		break;
	case AnimationState::SHOOT:
		PlayAnimation(entity, catShoot, 1.0f);
		break;

	case AnimationState::NONE:
		break;

	default:
		core::LogError("AnimationState Default, not supposed to happen !");
		break;
	}
}

}






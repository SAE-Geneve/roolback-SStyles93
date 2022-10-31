#include <fmt/format.h>
#include <filesystem>

#include "game/animation_manager.h"
#include "game/game_manager.h"

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
		sf::Texture newTexture{};
		const auto fullPath = fmt::format("data/sprites/{}/{}{}.png",path, path, i);
		if (!newTexture.loadFromFile(fullPath))
		{
			core::LogError(fmt::format("Could not load data/sprites/{}/{}{}.png sprite",path, path, i));
		}
		animation.textures.push_back(newTexture);
	}
}

void AnimationManager::LoopAnimation(const core::Entity& entity, const Animation& animation, AnimationData& animationData, const float speed) const
{
	auto& playerSprite = spriteManager_.GetComponent(entity);

	if (animationData.time >= ANIMATION_PERIOD / speed)
	{
		animationData.textureIdx++;
		if (animationData.textureIdx >= animation.textures.size())
		{
			//resets the animation (it loops)
			animationData.textureIdx = 0;
		}
		animationData.time = 0;
	}
	//Prevents texture index from being out of range of the textures vector
	if (animationData.textureIdx >= animation.textures.size())
	{
		animationData.textureIdx = animation.textures.size() - 1;
	}
	playerSprite.setTexture(animation.textures[animationData.textureIdx]);
}

void AnimationManager::PlayAnimation(const core::Entity& entity, const Animation& animation, AnimationData& animationData, const float speed) const
{
	auto& playerSprite = spriteManager_.GetComponent(entity);

	if (animationData.time >= ANIMATION_PERIOD / speed)
	{
		animationData.textureIdx++;
		if (animationData.textureIdx >= animation.textures.size())
		{
			//blocks the animation on last texture
			animationData.textureIdx = animation.textures.size() - 1;
		}
		animationData.time = 0;
	}
	if (animationData.textureIdx >= animation.textures.size())
	{
		animationData.textureIdx = animation.textures.size() - 1;
	}
	playerSprite.setTexture(animation.textures[animationData.textureIdx]);
}

void AnimationManager::UpdateEntity(core::Entity entity, AnimationState animationState, sf::Time dt)
{
	AnimationData& data = GetComponent(entity);
	data.time += dt.asSeconds();

	const auto playerCharacter = gameManager_.GetRollbackManager().GetPlayerCharacterManager().GetComponent(entity);

	if(data.animationState != playerCharacter.animationState)
	{
		data.textureIdx = 0;
	}

	switch (animationState)
	{
	case AnimationState::IDLE:
		data.animationState = AnimationState::IDLE;
		LoopAnimation(entity, catIdle, data, 1.0f);
		break;

	case AnimationState::WALK:
		data.animationState = AnimationState::WALK;
		LoopAnimation(entity, catWalk, data, 1.0f);
		break;

	case AnimationState::JUMP:
		data.animationState = AnimationState::JUMP;
		PlayAnimation(entity, catJump, data, 2.0f);
		break;

	case AnimationState::SHOOT:
		data.animationState = AnimationState::SHOOT;
		LoopAnimation(entity, catShoot, data, 1.0f);
		break;

	case AnimationState::NONE:
		break;

	default:
		core::LogError("AnimationState Default, not supposed to happen !");
		break;
	}
}

}






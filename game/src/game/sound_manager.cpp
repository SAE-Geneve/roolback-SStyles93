#include "game/sound_manager.h"
#include "game/game_manager.h"
#include <fmt/format.h>

namespace game
{
	SoundManager::SoundManager(core::EntityManager& entityManager, GameManager& gameManager) :
		ComponentManager(entityManager), gameManager_(gameManager) {}

	void SoundManager::LoadSound(const std::string_view path, Sound& sound) const
	{
		//LOAD SOUNDS
		const auto fullPath = fmt::format("data/sounds/{}.wav", path);
		if (!sound.soundBuffer.loadFromFile(fullPath))
		{
			core::LogError(fmt::format("Could not load data/sounds/{}.wav sound", path));
		}
	}

	void SoundManager::PlaySound(const core::Entity& entity)
	{
		auto& playerCharacter = gameManager_.GetRollbackManager().GetPlayerCharacterManager().GetComponent(entity);

		if (playerCharacter.animationState == AnimationState::JUMP && playerCharacter.isGrounded)
		{
			soundToPlay.setBuffer(catJumpSound.soundBuffer);
			if (soundToPlay.getStatus() == sf::Sound::Playing)
				return;
			soundToPlay.play();
		}
		if (playerCharacter.animationState == AnimationState::SHOOT && playerCharacter.isShooting)
		{
			soundToPlay.setBuffer(catShootSound.soundBuffer);
			if (soundToPlay.getStatus() == sf::Sound::Playing)
				return;
			soundToPlay.play();
		}
		if (playerCharacter.invincibilityTime == PLAYER_INVINCIBILITY_PERIOD)
		{
			soundToPlay.setBuffer(catHissSound.soundBuffer);
			if (soundToPlay.getStatus() == sf::Sound::Playing)
				return;
			soundToPlay.play();
		}
	}
}
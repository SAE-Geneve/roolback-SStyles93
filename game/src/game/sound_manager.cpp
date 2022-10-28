#include "game/sound_manager.h"
#include "game/game_manager.h"
#include <fmt/format.h>

namespace game
{
	SoundManager::SoundManager(core::EntityManager& entityManager, GameManager& gameManager) :
		ComponentManager(entityManager), gameManager_(gameManager) {}

	void SoundManager::LoadSound(const std::string_view path, SoundBuffer& soundBuffer) const
	{
		//LOAD SOUNDS
		const auto fullPath = fmt::format("data/sounds/{}.wav", path);
		if (!soundBuffer.soundBuffer.loadFromFile(fullPath))
		{
			core::LogError(fmt::format("Could not load data/sounds/{}.wav sound", path));
		}
	}

	void SoundManager::PlaySound(const core::Entity& entity)
	{
		auto& playerCharacter = gameManager_.GetRollbackManager().GetPlayerCharacterManager().GetComponent(entity);


		AudioSource& source = GetComponent(entity);
		source.sound.setVolume(GAME_VOLUME);

		if (source.sound.getStatus() == sf::Sound::Playing ||
			source.animationState == playerCharacter.animationState)
			return;
		
		if (playerCharacter.animationState == AnimationState::JUMP)
		{
			source.sound.setBuffer(catJumpSound.soundBuffer);
			source.sound.play();
			source.animationState = AnimationState::JUMP;
		}
		if (playerCharacter.animationState == AnimationState::SHOOT)
		{
			source.sound.setBuffer(catShootSound.soundBuffer);
			source.sound.play();
			source.animationState = AnimationState::SHOOT;
		}
		if (playerCharacter.invincibilityTime >= PLAYER_INVINCIBILITY_PERIOD)
		{
			source.sound.setBuffer(catHissSound.soundBuffer);
			source.sound.play();
		}
		if(playerCharacter.isGrounded)
		{
			source.animationState = AnimationState::NONE;
		}
	}
}
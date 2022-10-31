#pragma once
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "animation_manager.h"
#include "game_globals.h"

namespace game
{
    class GameManager;


    struct AudioSource
    {
        sf::Sound sound;
        AnimationState animationState = AnimationState::NONE;
    };
    struct SoundBuffer
    {
        sf::SoundBuffer soundBuffer;
    };
    /**
     * \brief SoundManager is a ComponentManager that holds all the sounds in one place.
     */
    class SoundManager : public core::ComponentManager<AudioSource, static_cast<core::EntityMask>(ComponentType::SOUND)>
    {

    public:

        SoundManager(core::EntityManager& entityManager, GameManager& gameManager);

        void LoadSound(std::string_view path, SoundBuffer& soundBuffer) const;

        void PlaySound(const core::Entity& entity);
        
        SoundBuffer catJumpSound;
        SoundBuffer catHissSound;

    private:
        GameManager& gameManager_;
    };
}

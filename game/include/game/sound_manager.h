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

        /**
         * @brief Loads a sound to a given soundbuffer
         * @param path The name of the sound in the folder (path)
         * @param soundBuffer The buffer to load the sound in
        */
        void LoadSound(std::string_view path, SoundBuffer& soundBuffer) const;
        /**
         * @brief Plays the sounds on entities
         * @param entity the given entity to play sounds on
        */
        void PlaySound(const core::Entity& entity);
        
        SoundBuffer catJumpSound;
        SoundBuffer catHissSound;

    private:
        GameManager& gameManager_;
    };
}

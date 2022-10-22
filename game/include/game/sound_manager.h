#pragma once
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "game_globals.h"

namespace game
{
    class GameManager;

    struct Sound
    {
        sf::SoundBuffer soundBuffer;
    };
    /**
     * \brief SoundManager is a ComponentManager that holds all the sounds in one place.
     */
    class SoundManager : public core::ComponentManager<Sound, static_cast<core::EntityMask>(ComponentType::SOUND)>
    {

    public:

        SoundManager(core::EntityManager& entityManager, GameManager& gameManager);

        void LoadSound(const std::string_view path, Sound& sound) const;

        void PlaySound(const core::Entity& entity);
        
        sf::Sound soundToPlay;
        
        Sound catJumpSound;
        Sound catHissSound;
        Sound catShootSound;

    private:
        GameManager& gameManager_;
    };
}

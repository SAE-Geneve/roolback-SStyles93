#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Text.hpp>

#include "game_globals.h"
#include "animation_manager.h"
#include "engine/entity.h"
#include "engine/system.h"
#include "engine/transform.h"
#include "graphics/graphics.h"
#include "graphics/sprite.h"
#include "network/packet_type.h"
#include "rollback_manager.h"
#include "sound_manager.h"

namespace game
{
class PacketSenderInterface;

/**
 * \brief GameManager is a class which manages the state of the game. It is shared between the client and the server.
 */
class GameManager
{
public:
    GameManager();
    virtual ~GameManager() = default;
    /**
     * @brief Spawns the player on the Game Manger
     * @param playerNumber The ID of the player to spawn
     * @param position the position at which we want to spawn him
     * @param direction The direction the player will be looking at when spawning
    */
    virtual void SpawnPlayer(PlayerNumber playerNumber, core::Vec2f position, core::Vec2f direction);
    virtual core::Entity SpawnBullet(PlayerNumber, core::Vec2f position, core::Vec2f velocity);
    virtual void DestroyBullet(core::Entity entity);
    [[nodiscard]] core::Entity GetEntityFromPlayerNumber(PlayerNumber playerNumber) const;
    [[nodiscard]] Frame GetCurrentFrame() const { return currentFrame_; }
    [[nodiscard]] Frame GetLastValidateFrame() const { return rollbackManager_.GetLastValidateFrame(); }
    [[nodiscard]] core::TransformManager& GetTransformManager() { return transformManager_; }
    [[nodiscard]] RollbackManager& GetRollbackManager() { return rollbackManager_; }
    virtual void SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame);
    /**
     * \brief Validate is a method called by the server to validate a frame.
     */
    void Validate(Frame newValidateFrame);
    [[nodiscard]] PlayerNumber CheckWinner() const;
    virtual void WinGame(PlayerNumber winner);


protected:
    core::EntityManager entityManager_;
    core::TransformManager transformManager_;
    RollbackManager rollbackManager_;
    std::array<core::Entity, MAX_PLAYER_NMB> playerEntityMap_{};
    Frame currentFrame_ = 0;
    PlayerNumber winner_ = INVALID_PLAYER;
};

/**
 * \brief ClientGameManager is a class that inherits from GameManager by adding the visual part and specific implementations needed by the clients.
 */
class ClientGameManager final : public GameManager,
                                public core::DrawInterface, public core::DrawImGuiInterface, public core::SystemInterface
{
public:

    enum State : std::uint32_t
    {
        STARTED = 1u << 0u,
        FINISHED = 1u << 1u,
    };
    explicit ClientGameManager(PacketSenderInterface& packetSenderInterface);
    /**
     * @brief Launches the begining of the game
     * @param startingTime The given time of launch
    */
    void StartGame(unsigned long long int startingTime);
    /**
     * @brief Instantiates all the elements for the game (sprites, sounds...)
    */
    void Begin() override;
    /**
     * @brief The graphical update of the game
     * @param dt The given delta time
    */
    void Update(sf::Time dt) override;
    /**
     * @brief Ends the game once conditions are met
    */
    void End() override;
    /**
     * @brief Sets the windowSize to the given size
     * @param windowsSize The desired window size
    */
    void SetWindowSize(sf::Vector2u windowsSize);
    [[nodiscard]] sf::Vector2u GetWindowSize() const { return windowSize_; }
    /**
     * @brief renders all the elements (sprites/texts)
     * @param target The target to render
    */
    void Draw(sf::RenderTarget& target) override;
    /**
     * @brief Sets the ClientID to the given playerNumber
     * @param clientPlayer The PlayerNumber to assign
    */
    void SetClientPlayer(PlayerNumber clientPlayer);
    void SpawnPlayer(PlayerNumber playerNumber, core::Vec2f position, core::Vec2f direction) override;
    /**
     * @brief Spawns a bullet
     * @param playerNumber The player's ID to give to the bullet
     * @param position The position at which we want to spawn a bullet
     * @param velocity The velocity to give to a bullet
     * @return Entity (bullet)
    */
    core::Entity SpawnBullet(PlayerNumber playerNumber, core::Vec2f position, core::Vec2f velocity) override;
    /**
     * @brief Creates a Healthbar for a player
     * @param playerNumber The player number for which we want to create a Healthbar
    */
    void CreateHealthBar(PlayerNumber playerNumber);
    /**
     * @brief The Physical update 
    */
    void FixedUpdate();
    /**
     * @brief Sets the players Input to the given input at a given frame
     * @param playerNumber The ID of the player to set
     * @param playerInput The input to give
     * @param inputFrame The frame at which we want to set the input
    */
    void SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame) override;
    /**
     * @brief The rendering of all Guis (ImGui)
    */
    void DrawImGui() override;
    /**
     * @brief Confirmation of a fram
     * @param newValidateFrame The new frame to validate
     * @param physicsStates the physics state given for check and validation
    */
    void ConfirmValidateFrame(Frame newValidateFrame, const std::array<PhysicsState, MAX_PLAYER_NMB>& physicsStates);
    [[nodiscard]] PlayerNumber GetPlayerNumber() const { return clientPlayer_; }
    /**
     * @brief Method used to declare when the game has been won
     * @param winner The player ID of the winner
    */
    void WinGame(PlayerNumber winner) override;
    [[nodiscard]] std::uint32_t GetState() const { return state_; }

protected:

    void UpdateCameraView();
    /**
     * @brief Loads the background sprites in the background texture vector
     * @param path The path that contains the sprites
    */
    void LoadBackground(std::string_view path);
    /**
     * @brief Instantiates the background elements
    */
    void CreateBackground();

    PacketSenderInterface& packetSenderInterface_;
    sf::Vector2u windowSize_;
    sf::View originalView_;
    sf::View cameraView_;
    PlayerNumber clientPlayer_ = INVALID_PLAYER;
    core::SpriteManager spriteManager_;
    float fixedTimer_ = 0.0f;
    unsigned long long startingTime_ = 0;
    std::uint32_t state_ = 0;

    AnimationManager animationManager_;
    sf::Texture bulletTexture_{};
	sf::Texture wallTexture_{};
    std::vector<sf::Texture> backgroundTextures_{};

    std::array<core::Entity, MAX_PLAYER_NMB>  healthBarMap{};

    SoundManager soundManager_;

    sf::Font font_;
    sf::Text textRenderer_;

	bool drawPhysics_ = false;
};
}

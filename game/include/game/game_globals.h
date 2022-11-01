/**
 * \file game_globals.h
 */

#pragma once
#include <SFML/Graphics/Color.hpp>
#include <array>

#include "engine/component.h"
#include "engine/entity.h"
#include "graphics/color.h"
#include "maths/angle.h"
#include "maths/vec2.h"


namespace game
{
    /**
     * \brief PlayerNumber is a type used to define the number of the player.
     * Starting from 0 to maxPlayerNmb
     */
    using PlayerNumber = std::uint8_t;
    /**
     * \brief INVALID_PLAYER is an integer constant that defines an invalid player number.
     */
    constexpr auto INVALID_PLAYER = std::numeric_limits<PlayerNumber>::max();
    /**
     * \brief ClientId is a type used to define the client identification.
     * It is given by the server to clients.
     */
    enum class ClientId : std::uint16_t {};
    constexpr auto INVALID_CLIENT_ID = ClientId{ 0 };
    using Frame = std::uint32_t;
    /**
     * \brief mmaxPlayerNmb is a integer constant that defines the maximum number of player per game
     */
    constexpr std::uint32_t MAX_PLAYER_NMB = 2;

    constexpr float PLAYER_HEALTH = 100.0f;
    constexpr float PLAYER_SPEED = 5.0f;
    constexpr float PLAYER_JUMP_FORCE = 10.0f;
    /**
     * @brief The period at which the player can shoot bullets
    */
    constexpr float PLAYER_SHOOTING_PERIOD = 0.5f;
    /**
     * @brief The speed at which the player charges his bullet
    */
    constexpr float PLAYER_CHARGE_SPEED = 0.8f;
    constexpr float PLAYER_INVINCIBILITY_PERIOD = 1.5f;
    constexpr float INVINCIBILITY_FLASH_PERIOD = 0.5f;
	/**
	 * @brief The time after which the animation texture changes
	*/
	constexpr float ANIMATION_PERIOD = 0.25f;
    
    constexpr float BULLET_SPEED = 5.0f;
    constexpr float BULLET_SCALE = 2.0f;
    constexpr float BULLET_PERIOD = 3.0f;
    constexpr float BULLET_ROTATION_SPEED = 500.0f;
    /**
     * @brief The maximum power that a bullet can have
    */
    constexpr float BULLET_MAX_POWER = 2.0f;
    /**
     * @brief The final power of the bullet on the player's life 
     * @brief (ex: 4.0f == 4 bullets to kill 100% of player's life
    */
    constexpr float BULLET_PER_LIFE_COEF = 4.0f;
    constexpr float BULLET_PUSH_POWER = 40.0f;

    constexpr  float GRAVITY = -9.81f;
    constexpr float UPPER_LIMIT = 8.0f;
    constexpr float RIGHT_LIMIT = 10.0f;
    constexpr float LOWER_LIMIT = -7.0f;
    constexpr float LEFT_LIMIT = -10.0f;

	constexpr core::Vec2f HEALTH_BAR_SCALE{ 4.0f, 0.5f };
    constexpr core::Vec2f PLAYER_SCALE{ 5.0f,5.0f };
    constexpr core::Color FLOOR_COLOR = core::Color(125, 180, 0, 255);

    constexpr float GAME_VOLUME = 20.0f;

/**
 * \brief windowBufferSize is the size of input stored by a client. 5 seconds of frame at 50 fps
 */
constexpr std::size_t WINDOW_BUFFER_SIZE = 5u * 50u;

/**
 * \brief startDelay is the delay to wait before starting a game in milliseconds
 */
constexpr long long START_DELAY = 3000;
/**
 * \brief maxInputNmb is the number of inputs stored into an PlayerInputPacket
 */
constexpr std::size_t MAX_INPUT_NMB = 50;
/**
 * \brief fixedPeriod is the period used in seconds to start a new FixedUpdate method in the game::GameManager
 */
constexpr float FIXED_PERIOD = 0.02f; //50fps


constexpr std::array<core::Color, std::max(4u, MAX_PLAYER_NMB)> PLAYER_COLORS
{
    core::Color::red(),
    core::Color::cyan(),
    core::Color::yellow(),
    core::Color::blue()
};

constexpr std::array<core::Vec2f, std::max(4u, MAX_PLAYER_NMB)> SPAWN_POSITIONS
{
    core::Vec2f(-2,-1),
    core::Vec2f(2,-1),
    core::Vec2f(-1,-1),
    core::Vec2f(1,-1),
};

constexpr std::array<core::Vec2f, std::max(4u, MAX_PLAYER_NMB)> SPAWN_DIRECTION
{
    core::Vec2f(1,0),
    core::Vec2f(-1,0),
    core::Vec2f(1,0),
    core::Vec2f(-1,0),
};

constexpr std::array<float, std::max(4u, MAX_PLAYER_NMB)> HEALTHBAR_POSITION
{
    -6.0f,
    4.0f,
    -0.0f,
    2.0f,
};
constexpr float HEALTH_BAR_HEIGHT = 8.0f;

enum class ComponentType : core::EntityMask
{
    BOX_COLLIDER = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE),
    DIRECTION = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 1u,
    PLAYER_CHARACTER = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE)<< 2u,
    BULLET = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 3u,
    ANIMATION = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 4u,
    SOUND = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 5u,
    DESTROYED = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 6u
};

/**
 * \brief PlayerInput is a type defining the input data from a player.
 */
using PlayerInput = std::uint8_t;

namespace PlayerInputEnum
{
enum PlayerInput : std::uint8_t
{
    NONE = 0u,
    UP = 1u << 0u,
    DOWN = 1u << 1u,
    LEFT = 1u << 2u,
    RIGHT = 1u << 3u,
    SHOOT = 1u << 4u,
};
}
}

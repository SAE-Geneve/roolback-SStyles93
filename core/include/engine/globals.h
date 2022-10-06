/**
 * \file globals.h
 */

#pragma once

#include <SFML/System/Vector2.hpp>
#include <cstdlib>

namespace core
{
const sf::Vector2u WINDOW_SIZE = { 1280, 720 };
constexpr std::size_t ENTITY_INIT_NMB = 128;
constexpr float PIXEL_PER_METER = 100.0f;
} // namespace core

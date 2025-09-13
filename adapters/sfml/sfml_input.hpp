#pragma once

#include "src/core/input.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>

namespace folio::adapters
{

class SfmlInput
{
public:
    core::InputState sample() const
    {
        core::InputState s{};
        s.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
        s.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
        s.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        s.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        s.dash = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
        return s;
    }
    bool attackPressedEdge(bool &prev) const
    {
        bool now = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        bool edge = now && !prev;
        prev = now;
        return edge;
    }
    bool facingRight(const sf::RenderWindow &win, float x) const
    {
        auto mp = sf::Mouse::getPosition(win);
        return (float)mp.x >= x;
    }
};

} // namespace folio::adapters

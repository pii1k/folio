#pragma once
#include "game/movement/character_controller.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>

namespace folio::adapters
{

class SfmlInput
{
public:
    movement::InputState sample() const
    {
        movement::InputState s{};
        s.up = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
        s.down = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        s.left = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        s.right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        s.dash = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
        return s;
    }
    bool attackPressedEdge(bool &prev) const
    {
        bool now = sf::Mouse::isButtonPressed(sf::Mouse::Left);
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

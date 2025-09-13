#pragma once

#include "apps/interface/game.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/VideoMode.hpp>

namespace folio::app
{
class GameLoop
{
public:
    explicit GameLoop(const Config &cfg)
        : window_(sf::VideoMode(sf::Vector2u{static_cast<unsigned>(cfg.width), static_cast<unsigned>(cfg.height)}), cfg.title)
    {
        window_.setFramerateLimit(cfg.frame_rate_limit);
        window_.setVerticalSyncEnabled(cfg.vsync);
    }

    sf::RenderWindow &window() { return window_; };
    void run(Game &game, const TickRates &rates = {});

private:
    sf::RenderWindow window_{};
};

} // namespace folio::app

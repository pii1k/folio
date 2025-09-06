#include "game_loop.hpp"
#include "apps/interface/game.hpp"

namespace folio::app
{
void GameLoop::run(Game &game, const TickRates &rates)
{
    AppContext ctx{};
    game.init(ctx);

    sf::Clock clock;
    float acc = 0.f;

    while (window_.isOpen())
    {
    }
};
} // namespace folio::app
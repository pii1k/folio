#include "game_loop.hpp"
#include "apps/interface/game.hpp"

namespace folio::app
{
void GameLoop::run(Game &game, const TickRates &rates)
{
    AppContext ctx{};
    ctx.window = &window_;
    game.init(ctx);

    sf::Clock clock;
    float acc = 0.f;

    while (window_.isOpen())
    {
        sf::Event e;
        while (window_.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
            {
                window_.close();
                break;
            }
            game.event(ctx, e);
        }

        float frame = clock.restart().asSeconds();
        acc += frame;

        int steps = 0;
        while (acc >= rates.fixed_delta && steps < rates.max_steps)
        {
            game.fixedUpdate(ctx, rates.fixed_delta);
            acc -= rates.fixed_delta;
            ++steps;
        }

        game.frameUpdate(ctx, frame);
        game.render(ctx);
    }

    game.shutdown(ctx);
}
; // NOLINT
} // namespace folio::app

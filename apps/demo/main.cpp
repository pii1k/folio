// Demo entry bootstraps the GameLoop with DemoGame
#include "apps/app_core/game_loop.hpp"
#include "demo_game.hpp"

int main()
{
    folio::app::Config cfg{};
    cfg.width = 960;
    cfg.height = 540;
    cfg.title = "folio demo - 2D Open World RPG Game PoC";
    cfg.frame_rate_limit = 120;

    folio::app::GameLoop loop(cfg);
    folio::demo::DemoGame game;
    loop.run(game);
    return 0;
}

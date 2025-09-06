#pragma once

#include <SFML/Window/Event.hpp>
#include <string>

namespace sf
{
class RenderWindow;
}

namespace folio::app
{
struct TickRates
{
    float fixed_delta{1.f / 120.f};
    int max_steps{5};
};

struct Config
{
    int width{960};
    int height{540};
    std::string title{"folio demo"};
    int frame_rate_limit{0}; // disable = 0
    bool vsync = false;
};

struct AppContext
{
    // TODO(jyan): 필요시 입력, 렌더러, 오디오 핸들 등 추가
    sf::RenderWindow *window{nullptr};
};

class Game
{
public:
    virtual ~Game() = default;
    virtual void init(AppContext &ctx) = 0;
    virtual void event(AppContext &ctx, const sf::Event &event) = 0;
    virtual void fixedUpdate(AppContext &ctx, float dt) = 0;
    virtual void frameUpdate(AppContext &ctx, float ft) = 0;
    virtual void render(AppContext &ctx) = 0;
    virtual void shutdown(AppContext &ctx) = 0;
};
} // namespace folio::app
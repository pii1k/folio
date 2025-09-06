#include "demo_game.hpp"
#include "src/core/utilities.hpp"
#include <SFML/Graphics.hpp>
#include <random>
#include <algorithm>
#include <cmath>

namespace folio::demo
{

using geometry::AABB;
using geometry::Transform;

void DemoGame::init(app::AppContext &ctx)
{
    // world
    const int TS = 32;
    map_ = makeOverworld("overworld", 180, 120, TS);
    world_bounds_ = world::boundsAABB(map_);
    chunks_ = std::make_unique<world::ChunkCache>(map_, 32);

    // player
    tr_.pos = {TS * 10.f, TS * 10.f};
    tr_.r = 12.f;

    // camera
    cam_ = sf::View(sf::FloatRect(0, 0, 960, 540));
}

void DemoGame::event(app::AppContext &ctx, const sf::Event &event)
{
    (void)ctx;
    (void)event;
}

void DemoGame::fixedUpdate(app::AppContext &ctx, float dt)
{
    // input
    in_ = input_.sample();
    if (ctx.window)
    {
        facing_right_ = input_.facingRight(*ctx.window, tr_.pos.x);
    }

    // move attempt
    const geometry::Vec2 prev = tr_.pos;
    ctrl_.tick(tr_, in_, folio::FixedDelta{dt}, world_bounds_);

    // collision resolve: AABB approx + axis rollback
    geometry::AABB me{tr_.pos.x - tr_.r, tr_.pos.y - tr_.r, tr_.r * 2, tr_.r * 2};
    bool hit = anyHit(me);
    if (hit)
    {
        tr_.pos.x = prev.x;
        me.x = tr_.pos.x - tr_.r;
        bool hitX = anyHit(me);
        if (hitX)
        {
            tr_.pos = prev;
            me.x = tr_.pos.x - tr_.r;
            me.y = tr_.pos.y - tr_.r;
        }
        else
        {
            tr_.pos.y = prev.y;
            me.y = tr_.pos.y - tr_.r;
        }
    }

    // prepare chunks
    chunks_->appendVisibleRange(cam_, jobs_);
    jobs_.drain(0.001); // small budget per fixed step
}

void DemoGame::frameUpdate(app::AppContext &ctx, float ft)
{
    (void)ft;
    const float hw = cam_.getSize().x * 0.5f;
    const float hh = cam_.getSize().y * 0.5f;
    const float cx = clampf(tr_.pos.x, world_bounds_.x + hw, world_bounds_.x + world_bounds_.w - hw);
    const float cy = clampf(tr_.pos.y, world_bounds_.y + hh, world_bounds_.y + world_bounds_.h - hh);
    cam_.setCenter(cx, cy);
}

void DemoGame::render(app::AppContext &ctx)
{
    auto &win = *ctx.window;
    win.setView(cam_);
    win.clear(sf::Color(28, 30, 34));

    // draw chunks
    chunks_->drawVisible(win, cam_);

    // player
    sf::CircleShape pc(tr_.r, 18);
    pc.setOrigin(tr_.r, tr_.r);
    pc.setPosition(tr_.pos.x, tr_.pos.y);
    pc.setFillColor(sf::Color(100, 200, 255));
    win.draw(pc);

    // facing line
    sf::Vertex line[2] = {
        sf::Vertex(sf::Vector2f(tr_.pos.x, tr_.pos.y), sf::Color(200, 200, 200)),
        sf::Vertex(sf::Vector2f(tr_.pos.x + (facing_right_ ? 18.f : -18.f), tr_.pos.y), sf::Color(200, 200, 200))};
    win.draw(line, 2, sf::Lines);

    // HUD
    win.setView(win.getDefaultView());
    sf::RectangleShape bar({240.f, 10.f});
    bar.setPosition(16.f, 16.f);
    bar.setFillColor(sf::Color(90, 200, 120));
    win.draw(bar);
}

void DemoGame::shutdown(app::AppContext &ctx)
{
    (void)ctx;
}

world::TileMap DemoGame::makeOverworld(const std::string &id, int W, int H, int tile_size)
{
    world::TileMap m;
    m.id = id;
    m.w = W;
    m.h = H;
    m.tile_size = tile_size;
    m.tiles.assign(W * H, 0);

    // borders walls
    for (int x = 0; x < W; ++x)
    {
        m.tiles[x] = 1;
        m.tiles[(H - 1) * W + x] = 1;
    }
    for (int y = 0; y < H; ++y)
    {
        m.tiles[y * W] = 1;
        m.tiles[y * W + (W - 1)] = 1;
    }

    // cross roads
    for (int x = 2; x < W - 2; ++x)
        m.tiles[(H / 2) * W + x] = 0;
    for (int y = 2; y < H - 2; ++y)
        m.tiles[y * W + (W / 2)] = 0;

    // random clusters
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> rx(2, W - 3), ry(2, H - 3), rs(2, 7);
    for (int i = 0; i < 120; ++i)
    {
        int cx = rx(rng), cy = ry(rng), sx = rs(rng), sy = rs(rng);
        for (int y = cy; y < std::min(H - 2, cy + sy); ++y)
            for (int x = cx; x < std::min(W - 2, cx + sx); ++x)
                m.tiles[y * W + x] = 1;
    }

    // build colliders
    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            if (m.tiles[y * W + x] == 1)
            {
                m.colliders.push_back(geometry::AABB{float(x * tile_size), float(y * tile_size), float(tile_size), float(tile_size)});
            }
        }
    }
    return m;
}

bool DemoGame::aabbOverlap(const geometry::AABB &a, const geometry::AABB &b) const
{
    return !(a.x + a.w <= b.x || b.x + b.w <= a.x || a.y + a.h <= b.y || b.y + b.h <= a.y);
}

bool DemoGame::anyHit(const geometry::AABB &box) const
{
    const int TS = map_.tile_size;
    const int minX = std::max(0, int(std::floor(box.x / TS)));
    const int maxX = std::min(map_.w - 1, int(std::floor((box.x + box.w) / TS)));
    const int minY = std::max(0, int(std::floor(box.y / TS)));
    const int maxY = std::min(map_.h - 1, int(std::floor((box.y + box.h) / TS)));
    for (int ty = minY; ty <= maxY; ++ty)
    {
        for (int tx = minX; tx <= maxX; ++tx)
        {
            if (!map_.isWall(tx, ty))
                continue;
            geometry::AABB aabb{float(tx * TS), float(ty * TS), float(TS), float(TS)};
            if (aabbOverlap(box, aabb))
                return true;
        }
    }
    return false;
}

} // namespace folio::demo

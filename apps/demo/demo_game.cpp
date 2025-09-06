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
    iso_ = world::IsoDims{float(TS * 2), float(TS)}; // typical diamond w:h = 2:1
    iso_bounds_ = world::isoMapBounds(map_, iso_);
    chunks_ = std::make_unique<world::ChunkCache>(map_, 32);
    chunks_->setIsometric(iso_);

    // player
    tr_.pos = {TS * 10.f, TS * 10.f};
    tr_.r = 12.f;

    // camera
    cam_ = sf::View(sf::FloatRect(0, 0, 960, 540));

    // warm up chunks for initial view
    chunks_->appendVisibleRange(cam_, jobs_);
    jobs_.drain();
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

    // map screen input to world-space direction for isometric equalized speed
    geometry::Vec2 screen_dir{(in_.right ? 1.f : 0.f) - (in_.left ? 1.f : 0.f),
                              (in_.down ? 1.f : 0.f) - (in_.up ? 1.f : 0.f)};
    screen_dir = geometry::norm(screen_dir);
    geometry::Vec2 world_dir{0.f, 0.f};
    if (geometry::len(screen_dir) > 0.f)
    {
        const float sx = iso_.w * 0.5f;
        const float sy = iso_.h * 0.5f;
        // inverse of iso projection matrix to get world direction from screen vector
        float fx = 0.5f * (screen_dir.x / sx + screen_dir.y / sy);
        float fy = 0.5f * (-screen_dir.x / sx + screen_dir.y / sy);
        world_dir = geometry::norm({fx, fy});
    }

    // move attempt using isometric-aware direction
    const geometry::Vec2 prev = tr_.pos;
    ctrl_.tickIso(tr_, in_, world_dir, folio::FixedDelta{dt}, world_bounds_);

    // collision resolve: AABB approx + axis rollback
    // collision resolve: axis-wise separate (X then Y) to avoid full stop on touch
    geometry::AABB me{tr_.pos.x - tr_.r, tr_.pos.y - tr_.r, tr_.r * 2, tr_.r * 2};
    // resolve X
    geometry::AABB meX{tr_.pos.x - tr_.r, prev.y - tr_.r, tr_.r * 2, tr_.r * 2};
    if (anyHit(meX))
    {
        tr_.pos.x = prev.x;
    }
    // resolve Y with possibly corrected X
    geometry::AABB meY{tr_.pos.x - tr_.r, tr_.pos.y - tr_.r, tr_.r * 2, tr_.r * 2};
    if (anyHit(meY))
    {
        tr_.pos.y = prev.y;
    }

    // prepare chunks
    chunks_->appendVisibleRange(cam_, jobs_);
    jobs_.drain(0.001); // small budget per fixed step
}

void DemoGame::frameUpdate(app::AppContext &ctx, float ft)
{
    (void)ft;
    // camera follows player in isometric space and clamps to iso map bounds
    const auto isoPos = world::worldToIso(tr_.pos.x, tr_.pos.y, map_.tile_size, iso_);
    const float hw = cam_.getSize().x * 0.5f;
    const float hh = cam_.getSize().y * 0.5f;
    const float cx = clampf(isoPos.x, iso_bounds_.x + hw, iso_bounds_.x + iso_bounds_.w - hw);
    const float cy = clampf(isoPos.y, iso_bounds_.y + hh, iso_bounds_.y + iso_bounds_.h - hh);
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
    // draw player at projected isometric position
    const auto ip = world::worldToIso(tr_.pos.x, tr_.pos.y, map_.tile_size, iso_);
    sf::CircleShape pc(tr_.r, 18);
    pc.setOrigin(tr_.r, tr_.r);
    pc.setPosition(ip.x, ip.y);
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

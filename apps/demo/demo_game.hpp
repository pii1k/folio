#pragma once

#include "apps/interface/game.hpp"
#include "adapters/sfml/sfml_input.hpp"
#include "src/core/input.hpp"
#include "src/concurrency/job_system.hpp"
#include "src/geometry/types.hpp"
#include "src/movement/character_controller.hpp"
#include "src/world/chunks.hpp"
#include "src/world/tile_map.hpp"
#include <memory>

namespace folio::demo
{

class DemoGame : public app::Game
{
public:
    void init(app::AppContext &ctx) override;
    void event(app::AppContext &ctx, const sf::Event &event) override;
    void fixedUpdate(app::AppContext &ctx, float dt) override;
    void frameUpdate(app::AppContext &ctx, float ft) override;
    void render(app::AppContext &ctx) override;
    void shutdown(app::AppContext &ctx) override;

private:
    world::TileMap makeOverworld(const std::string &id, int W, int H, int tile_size);
    bool aabbOverlap(const geometry::AABB &a, const geometry::AABB &b) const;
    bool anyHit(const geometry::AABB &box) const;

private:
    adapters::SfmlInput input_{};
    movement::CharacterController ctrl_{};
    core::InputState in_{};
    geometry::Transform tr_{};
    bool facing_right_{true};

    world::TileMap map_{};
    std::unique_ptr<world::ChunkCache> chunks_{};
    geometry::AABB world_bounds_{};
    sf::View cam_{};
    concurrency::JobSystem jobs_{1};
};

} // namespace folio::demo

#pragma once

#include "game/concurrency/job_system.hpp"
#include "game/geometry/types.hpp"
#include "tile_map.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/View.hpp>
#include <unordered_map>
#include <unordered_set>

namespace folio::world
{
struct ChunkKey
{
    bool operator==(const ChunkKey &chunk_key) const
    {
        return x == chunk_key.x && y == chunk_key.y;
    };

    int x, y;
};

struct ChunkKeyHash
{
    size_t operator()(const ChunkKey &chunk_key) const
    {
        return (chunk_key.x * 73856093u) ^ (chunk_key.y * 19349663u);
    };
};

class ChunkCache
{
public:
    ChunkCache(const TileMap &map, int chunk_tiles = 32)
        : tile_map_(map), chunk_(chunk_tiles) {}

    void ensureVisibleRange(const sf::View &cam, concurrency::JobSystem &jobs)
    {
    }

private:
    void visibleRange(const sf::View &cam, auto &&fn) const
    {
        const int tile_size = tile_map_.tile_size;
        const int chunk = chunk_;

        auto camera_center = cam.getCenter();
        auto camera_size = cam.getSize();
        float left = camera_center.x - camera_size.x * 0.5f,
              top = camera_center.y - camera_size.y * 0.5f,
              right = camera_center.x + camera_size.x * 0.5f,
              bottom = camera_center.y + camera_size.y * 0.5f;
    }

private:
    const TileMap &tile_map_;
    int chunk_;
    std::unordered_map<ChunkKey, sf::VertexArray, ChunkKeyHash> cache_;
    std::unordered_set<ChunkKey, ChunkKeyHash> pending_;
};

} // namespace folio::world
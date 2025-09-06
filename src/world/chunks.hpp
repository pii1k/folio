#pragma once

#include "src/concurrency/job_system.hpp"
#include "src/geometry/types.hpp"
#include "tile_map.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>
#include <algorithm>
#include <cmath>
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

    // 보이는 청크를 큐에 추가하고, 준비되지 않은 청크는 jobs로 베이크를 제출
    void appendVisibleRange(const sf::View &cam, concurrency::JobSystem &jobs)
    {
        visibleRange(cam, [&](const ChunkKey &key) {
            if (cache_.find(key) == cache_.end() && pending_.find(key) == pending_.end())
            {
                pending_.insert(key);
                jobs.submit([this, key]() {
                    auto va = buildChunk(key);
                    cache_[key] = std::move(va);
                    pending_.erase(key);
                });
            }
        });
    }

    void drawVisible(sf::RenderTarget &target, const sf::View &cam) const
    {
        visibleRange(cam, [&](const ChunkKey &key) {
            auto it = cache_.find(key);
            if (it != cache_.end())
            {
                target.draw(it->second);
            }
        });
    }

private:
    void visibleRange(const sf::View &cam, auto &&fn) const
    {
        const int tile_size = tile_map_.tile_size;
        const int chunk_tiles = chunk_;

        const auto cc = cam.getCenter();
        const auto cs = cam.getSize();
        const float left = cc.x - cs.x * 0.5f;
        const float top = cc.y - cs.y * 0.5f;
        const float right = cc.x + cs.x * 0.5f;
        const float bottom = cc.y + cs.y * 0.5f;

        const int chunk_world = chunk_tiles * tile_size;
        const int max_cx = std::max(1, (tile_map_.w + chunk_tiles - 1) / chunk_tiles);
        const int max_cy = std::max(1, (tile_map_.h + chunk_tiles - 1) / chunk_tiles);

        int cx0 = std::clamp(int(std::floor(left / chunk_world)) - 1, 0, max_cx - 1);
        int cy0 = std::clamp(int(std::floor(top / chunk_world)) - 1, 0, max_cy - 1);
        int cx1 = std::clamp(int(std::floor(right / chunk_world)) + 1, 0, max_cx - 1);
        int cy1 = std::clamp(int(std::floor(bottom / chunk_world)) + 1, 0, max_cy - 1);

        for (int cy = cy0; cy <= cy1; ++cy)
        {
            for (int cx = cx0; cx <= cx1; ++cx)
            {
                fn(ChunkKey{cx, cy});
            }
        }
    }

    sf::VertexArray buildChunk(const ChunkKey &key) const
    {
        const int ts = tile_map_.tile_size;
        const int cx = key.x * chunk_;
        const int cy = key.y * chunk_;
        const int ex = std::min(tile_map_.w, cx + chunk_);
        const int ey = std::min(tile_map_.h, cy + chunk_);

        sf::VertexArray va(sf::Quads);
        for (int y = cy; y < ey; ++y)
        {
            for (int x = cx; x < ex; ++x)
            {
                const int v = tile_map_.tiles[y * tile_map_.w + x];
                const sf::Color color = (v == 1) ? sf::Color(70, 75, 85)
                                                 : sf::Color(46, 52, 64);
                const float px = float(x * ts);
                const float py = float(y * ts);
                va.append(sf::Vertex({px, py}, color));
                va.append(sf::Vertex({px + ts, py}, color));
                va.append(sf::Vertex({px + ts, py + ts}, color));
                va.append(sf::Vertex({px, py + ts}, color));
            }
        }
        return va;
    }

private:
    const TileMap &tile_map_;
    int chunk_;
    std::unordered_map<ChunkKey, sf::VertexArray, ChunkKeyHash> cache_;
    std::unordered_set<ChunkKey, ChunkKeyHash> pending_;
};

} // namespace folio::world

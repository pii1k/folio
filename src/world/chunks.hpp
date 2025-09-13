#pragma once

#include "src/concurrency/job_system.hpp"
#include "src/geometry/types.hpp"
#include "tile_map.hpp"
#include "iso.hpp"
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

    void setIsometric(IsoDims dims)
    {
        isometric_ = true;
        iso_ = dims;
    }

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

    void invalidateTile(int tx, int ty)
    {
        const int cx = std::clamp(tx / chunk_, 0, std::max(1, (tile_map_.w + chunk_ - 1) / chunk_) - 1);
        const int cy = std::clamp(ty / chunk_, 0, std::max(1, (tile_map_.h + chunk_ - 1) / chunk_) - 1);
        ChunkKey key{cx, cy};
        cache_.erase(key);
        pending_.erase(key);
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

        const int max_cx = std::max(1, (tile_map_.w + chunk_tiles - 1) / chunk_tiles);
        const int max_cy = std::max(1, (tile_map_.h + chunk_tiles - 1) / chunk_tiles);

        int cx0, cy0, cx1, cy1;
        if (!isometric_)
        {
            const int chunk_world = chunk_tiles * tile_size;
            cx0 = std::clamp(int(std::floor(left / chunk_world)) - 1, 0, max_cx - 1);
            cy0 = std::clamp(int(std::floor(top / chunk_world)) - 1, 0, max_cy - 1);
            cx1 = std::clamp(int(std::floor(right / chunk_world)) + 1, 0, max_cx - 1);
            cy1 = std::clamp(int(std::floor(bottom / chunk_world)) + 1, 0, max_cy - 1);
        }
        else
        {
            // transform iso view rect corners back to world, then to fractional tile indices
            auto pLT = isoToWorld(left, top, tile_size, iso_);
            auto pRT = isoToWorld(right, top, tile_size, iso_);
            auto pRB = isoToWorld(right, bottom, tile_size, iso_);
            auto pLB = isoToWorld(left, bottom, tile_size, iso_);

            const float fx_min = std::min(std::min(pLT.x, pRT.x), std::min(pRB.x, pLB.x)) / float(tile_size);
            const float fx_max = std::max(std::max(pLT.x, pRT.x), std::max(pRB.x, pLB.x)) / float(tile_size);
            const float fy_min = std::min(std::min(pLT.y, pRT.y), std::min(pRB.y, pLB.y)) / float(tile_size);
            const float fy_max = std::max(std::max(pLT.y, pRT.y), std::max(pRB.y, pLB.y)) / float(tile_size);

            cx0 = std::clamp(int(std::floor(fx_min / chunk_tiles)) - 1, 0, max_cx - 1);
            cy0 = std::clamp(int(std::floor(fy_min / chunk_tiles)) - 1, 0, max_cy - 1);
            cx1 = std::clamp(int(std::floor(fx_max / chunk_tiles)) + 1, 0, max_cx - 1);
            cy1 = std::clamp(int(std::floor(fy_max / chunk_tiles)) + 1, 0, max_cy - 1);
        }

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

        sf::VertexArray va(sf::PrimitiveType::Triangles);
        if (!isometric_)
        {
            for (int y = cy; y < ey; ++y)
            {
                for (int x = cx; x < ex; ++x)
                {
                    const int v = tile_map_.tiles[y * tile_map_.w + x];
                    const sf::Color color = (v == 1) ? sf::Color(70, 75, 85)
                                                     : sf::Color(46, 52, 64);
                    const float px = float(x * ts);
                    const float py = float(y * ts);
                    // two triangles per tile
                    va.append(sf::Vertex(sf::Vector2f{px, py}, color));
                    va.append(sf::Vertex(sf::Vector2f{px + ts, py}, color));
                    va.append(sf::Vertex(sf::Vector2f{px + ts, py + ts}, color));
                    va.append(sf::Vertex(sf::Vector2f{px, py}, color));
                    va.append(sf::Vertex(sf::Vector2f{px + ts, py + ts}, color));
                    va.append(sf::Vertex(sf::Vector2f{px, py + ts}, color));
                }
            }
        }
        else
        {
            const float half_w = iso_.w * 0.5f;
            const float half_h = iso_.h * 0.5f;
            for (int y = cy; y < ey; ++y)
            {
                for (int x = cx; x < ex; ++x)
                {
                    const int v = tile_map_.tiles[y * tile_map_.w + x];
                    const sf::Color color = (v == 1) ? sf::Color(70, 75, 85)
                                                     : sf::Color(46, 52, 64);
                    auto top = tileToIso(x, y, iso_);
                    sf::Vector2f p0{top.x, top.y};
                    sf::Vector2f p1{top.x + half_w, top.y + half_h};
                    sf::Vector2f p2{top.x, top.y + iso_.h};
                    sf::Vector2f p3{top.x - half_w, top.y + half_h};
                    // two triangles to form a diamond
                    va.append(sf::Vertex(p0, color));
                    va.append(sf::Vertex(p1, color));
                    va.append(sf::Vertex(p2, color));
                    va.append(sf::Vertex(p0, color));
                    va.append(sf::Vertex(p2, color));
                    va.append(sf::Vertex(p3, color));
                }
            }
        }
        return va;
    }

private:
    const TileMap &tile_map_;
    int chunk_;
    bool isometric_{false};
    IsoDims iso_{};
    std::unordered_map<ChunkKey, sf::VertexArray, ChunkKeyHash> cache_;
    std::unordered_set<ChunkKey, ChunkKeyHash> pending_;
};

} // namespace folio::world

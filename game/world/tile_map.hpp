#pragma once

#include "game/geometry/types.hpp"
#include <string>
#include <vector>

namespace folio::world
{

struct TileMap
{
    std::string id{};
    int w{0}, h{0}, tile_size{32};
    std::vector<int> tiles;                // 0 바닥, 1 벽
    std::vector<geometry::AABB> colliders; // 벽 타일 AABB

    bool isWall(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= w || y >= h)
        {
            return true;
        }

        return tiles[y * w + x] == 1;
    }
};

inline geometry::AABB boundsAABB(const TileMap &map)
{
    return {0.f, 0.f, float(map.w * map.tile_size), float(map.h * map.tile_size)};
}

TileMap fromASCII(const std::string &id, int tile_size, const std::vector<std::string> &rows);
}; // namespace folio::world
#pragma once

#include "src/geometry/types.hpp"
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

inline geometry::AABB tileAABB(const TileMap &map, int tx, int ty)
{
    const float ts = float(map.tile_size);
    return {tx * ts, ty * ts, ts, ts};
}

inline std::pair<int, int> worldToTile(const TileMap &map, float wx, float wy)
{
    const int tx = int(std::floor(wx / float(map.tile_size)));
    const int ty = int(std::floor(wy / float(map.tile_size)));
    return {tx, ty};
}

TileMap fromASCII(const std::string &id, int tile_size, const std::vector<std::string> &rows);
}; // namespace folio::world

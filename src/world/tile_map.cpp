#include "tile_map.hpp"
#include "src/geometry/types.hpp"

namespace folio::world
{
TileMap fromASCII(const std::string &id, int tile_size, const std::vector<std::string> &rows)
{
    TileMap map{};
    map.id = id;
    map.tile_size = tile_size;
    map.h = int(rows.size());
    map.w = rows.empty() ? 0
                         : int(rows.front().size());
    map.tiles.resize(map.w * map.h, 0);
    for (int y = 0; y < map.h; ++y)
    {
        for (int x = 0; x < map.w; ++x)
        {
            char c = rows[y][x];
            int v = (c == '#') ? 1 : 0; // # 벽, . 바닥
            map.tiles[y * map.w + x] = v;
            if (v == 1)
            {
                map.colliders.push_back(geometry::AABB{float(x * tile_size), float(y * tile_size), float(tile_size), float(tile_size)});
            }
        }
    }

    return map;
}
} // namespace folio::world

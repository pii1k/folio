#pragma once

#include "tile_map.hpp"
#include <unordered_map>

namespace folio::world
{
struct World
{
    std::unordered_map<std::string, TileMap> maps{};
    std::string current_map_id;
};

} // namespace folio::world

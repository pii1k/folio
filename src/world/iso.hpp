#pragma once

#include "src/geometry/types.hpp"
#include "tile_map.hpp"
#include <algorithm>

namespace folio::world
{
struct IsoDims
{
    float w{64.f}; // diamond width in pixels
    float h{32.f}; // diamond height in pixels
};

inline geometry::Vec2 worldToIso(float wx, float wy, int tile_size, IsoDims iso)
{
    const float fx = wx / float(tile_size);
    const float fy = wy / float(tile_size);
    const float sx = iso.w * 0.5f;
    const float sy = iso.h * 0.5f;
    return {(fx - fy) * sx, (fx + fy) * sy};
}

inline geometry::Vec2 tileToIso(int tx, int ty, IsoDims iso)
{
    const float sx = iso.w * 0.5f;
    const float sy = iso.h * 0.5f;
    return {(tx - ty) * sx, (tx + ty) * sy};
}

inline geometry::Vec2 isoToWorld(float ix, float iy, int tile_size, IsoDims iso)
{
    const float sx = iso.w * 0.5f;
    const float sy = iso.h * 0.5f;
    const float fx = (ix / sx + iy / sy) * 0.5f;
    const float fy = (iy / sy - ix / sx) * 0.5f;
    return {fx * tile_size, fy * tile_size};
}

inline geometry::AABB isoMapBounds(const TileMap &map, IsoDims iso)
{
    // project the 4 tile corners (0,0), (w,0), (0,h), (w,h)
    auto p00 = tileToIso(0, 0, iso);
    auto pw0 = tileToIso(map.w, 0, iso);
    auto p0h = tileToIso(0, map.h, iso);
    auto pwh = tileToIso(map.w, map.h, iso);
    float minx = std::min(std::min(p00.x, pw0.x), std::min(p0h.x, pwh.x));
    float maxx = std::max(std::max(p00.x, pw0.x), std::max(p0h.x, pwh.x));
    float miny = std::min(std::min(p00.y, pw0.y), std::min(p0h.y, pwh.y));
    float maxy = std::max(std::max(p00.y, pw0.y), std::max(p0h.y, pwh.y));
    return {minx, miny, maxx - minx, maxy - miny};
}
} // namespace folio::world


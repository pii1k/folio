#pragma once

#include "src/geometry/types.hpp"

// TODO(jyan): 나중에 player로 ECS 분리. 지금은 Component만 추가함
namespace folio::combat
{

enum class Team
{
    Player,
    Enemy
};

struct Fighter
{
    Team team{Team::Player};
    float hp{120.f};
    float atk{16.f};
    float atk_range{34.f};
    float windup{0.12f};
    float recover{0.22f};
};

struct HitBox
{
    float cx, cy, w, h;
    Team from{Team::Player};
    float dmg{10.f};
};

inline folio::geometry::AABB makeSlashBox(const folio::geometry::Transform &tr, bool facingRight, float range, float heightScale = 1.2f)
{
    using namespace folio::geometry;
    Vec2 dir = facingRight ? Vec2{1.f, 0.f}
                           : Vec2{-1.f, 0.f};
    Vec2 center = tr.pos + dir * (tr.r + range * 0.5f);
    float w = range, h = tr.r * heightScale;
    return {center.x - w * 0.5f, center.y - h * 0.5f, w, h};
}

} // namespace folio::combat

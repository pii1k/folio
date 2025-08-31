#pragma once

#include <SFML/Graphics/Glsl.hpp>
#include <algorithm>
#include <cmath>

namespace folio::geometry
{
struct Vec2
{
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2 &o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2 &o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 &operator+=(const Vec2 &o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    float x{0}, y{0};
};

inline float len(const Vec2 &v) { return std::sqrt(v.x * v.x + v.y * v.y); }
inline Vec2 norm(const Vec2 &v)
{
    float length = len(v);
    return (length > 1e-5f) ? Vec2{v.x / length, v.y / length}
                            : Vec2{0, 0}; // maigc num for demo
}

struct Transform
{
    Vec2 pos{0, 0};
    float r{14.f}; // maigc num for demo
};

struct AABB
{
    float x, y, w, h;
};

inline bool circleAabb(const Vec2 &circle, float r, const AABB &a)
{
    float nx = std::max(a.x, std::min(circle.x, a.x + a.w));
    float ny = std::max(a.y, std::min(circle.y, a.y + a.h));
    float dx = circle.x - nx, dy = circle.y - ny;
    return dx * dx + dy * dy <= r * r;
}
} // namespace folio::geometry
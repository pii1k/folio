#pragma once

inline float clampf(float v, float a, float b)
{
    return (v < a) ? a : (v > b ? b : v);
}

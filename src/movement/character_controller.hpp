#pragma once

#include "src/core/time.hpp"
#include "src/core/input.hpp"
#include "src/geometry/types.hpp"

namespace folio::movement
{


struct MoveParams
{
    float walk_speed = 180.f;
    float dash_speed = 520.f;
    float dash_time = 0.15f;
    float stamina_max = 100.f;
    float stamina_regen = 20.f; // per second
    float dash_cost = 0.f;
};

struct MoveRuntime
{
    float dash_remain = 0.f;
    float stamina = 0.f;
};

class CharacterController
{
public:
    explicit CharacterController(MoveParams p = {}) : p_(p)
    {
        rt_.stamina = p_.stamina_max;
    }

    void tick(geometry::Transform &tr,
              const core::InputState &in,
              const FixedDelta &dt,
              const geometry::AABB &bounds);

    // Isometric or custom control: supply world-space direction directly
    void tickIso(geometry::Transform &tr,
                 const core::InputState &in,
                 const geometry::Vec2 &world_dir,
                 const FixedDelta &dt,
                 const geometry::AABB &bounds);

    const MoveRuntime &runtime() const { return rt_; }

private:
    MoveParams p_;
    MoveRuntime rt_;
};
} // namespace folio::movement

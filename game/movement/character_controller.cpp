#include "character_controller.hpp"

namespace folio::movement
{
void CharacterController::tick(geometry::Transform &transform,
                               const InputState &in,
                               const FixedDelta &dt,
                               const geometry::AABB &bounds)
{
    // 입력 벡터
    geometry::Vec2 direction{(in.right ? 1.f : 0.f) - (in.left ? 1.f : 0.f),
                             (in.down ? 1.f : 0.f) - (in.up ? 1.f : 0.f)};
    direction = norm(direction);

    // 대시 시작
    if (in.dash &&
        this->rt_.dash_remain <= 0.f &&
        this->rt_.stamina >= p_.dash_cost)
    {
        this->rt_.dash_remain = p_.dash_time;
        this->rt_.stamina -= p_.dash_cost;
        if (len(direction) < 0.1f)
        {
            direction = geometry::Vec2{1.f, 0.f}; // 정지 중 대시는 전방 가정(우측)
        }
    }

    // 속도 계산
    float speed = (this->rt_.dash_remain > 0.f) ? p_.dash_speed
                                                : p_.walk_speed;
    transform.pos += direction * (speed * dt.sec);

    // 대시 시간 감소
    if (this->rt_.dash_remain > 0.f)
    {
        this->rt_.dash_remain -= dt.sec;
    }

    // 경계 충돌(벽 클램프)
    transform.pos.x = clampf(transform.pos.x, bounds.x + transform.r,
                             bounds.x + bounds.w - transform.r);
    transform.pos.y = clampf(transform.pos.y, bounds.y + transform.r,
                             bounds.y + bounds.h - transform.r);
}

} // namespace folio::movement
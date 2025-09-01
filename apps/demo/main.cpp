#include <SFML/Graphics.hpp>
#include <random>

#include "adapters/sfml/sfml_input.hpp"
#include "game/core/time.hpp"
#include "game/geometry/types.hpp"
#include "game/movement/character_controller.hpp"
#include "game/world/tile_map.hpp"

namespace
{
using namespace folio;

// TODO(jyan): 나중에 맵 생성 관련 유틸 함수 분리
static world::TileMap makeOverworld(const std::string &id, int W, int H, int tile_size)
{
    world::TileMap m;
    m.id = id;
    m.w = W;
    m.h = H;
    m.tile_size = tile_size;
    m.tiles.assign(W * H, 0);

    // 1) 경계는 전부 벽
    for (int x = 0; x < W; ++x)
    {
        m.tiles[x] = 1;
        m.tiles[(H - 1) * W + x] = 1;
    }
    for (int y = 0; y < H; ++y)
    {
        m.tiles[y * W] = 1;
        m.tiles[y * W + (W - 1)] = 1;
    }

    // 2) 간단한 길(도로) 뚫기: 중앙 수평 & 수직
    for (int x = 2; x < W - 2; ++x)
        m.tiles[(H / 2) * W + x] = 0;
    for (int y = 2; y < H - 2; ++y)
        m.tiles[y * W + (W / 2)] = 0;

    // 3) 랜덤 벽 클러스터
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> rx(2, W - 3), ry(2, H - 3), rs(2, 7);
    for (int i = 0; i < 120; ++i)
    {
        int cx = rx(rng), cy = ry(rng), sx = rs(rng), sy = rs(rng);
        for (int y = cy; y < std::min(H - 2, cy + sy); ++y)
            for (int x = cx; x < std::min(W - 2, cx + sx); ++x)
                m.tiles[y * W + x] = 1;
    }

    // 4) 콜라이더 구축
    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            if (m.tiles[y * W + x] == 1)
            {
                m.colliders.push_back(geometry::AABB{float(x * tile_size), float(y * tile_size), float(tile_size), float(tile_size)});
            }
        }
    }
    return m;
}

static bool aabbOverlap(const geometry::AABB &a, const geometry::AABB &b)
{
    return !(a.x + a.w <= b.x || b.x + b.w <= a.x || a.y + a.h <= b.y || b.y + b.h <= a.y);
}

} // namespace

int main()
{
    sf::RenderWindow win(sf::VideoMode(960, 540), "folio demo - 2D Open World RPG Game PoC");
    win.setFramerateLimit(120);

    // map
    const int TS = 32;
    world::TileMap map = makeOverworld("overworld", 180, 120, TS); // 5760x3840 px
    geometry::AABB worldBounds = world::boundsAABB(map);

    // 플레이어
    geometry::Transform tr;
    tr.pos = {TS * 10.f, TS * 10.f};
    tr.r = 12.f;
    movement::CharacterController ctrl;
    adapters::SfmlInput input;
    movement::InputState in{};
    bool facingRight = true;

    sf::View cam(sf::FloatRect(0, 0, 960, 540));

    sf::Clock clock;
    float acc = 0.f;
    const float dt = 1.f / 120.f;

    while (win.isOpen())
    {
        sf::Event e;
        while (win.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                win.close();
        }

        float frame = clock.restart().asSeconds();
        acc += frame;

        while (acc >= dt)
        {
            acc -= dt;

            // 입력
            in = input.sample();
            facingRight = input.facingRight(win, tr.pos.x);

            // 이동 시도 (월드 경계 클램프는 ctrl.tick에서 1차 수행)
            geometry::Vec2 prev = tr.pos;
            ctrl.tick(tr, in, FixedDelta{dt}, worldBounds);

            // 타일 충돌 보정 (AABB 근사 + 축별 롤백)
            geometry::AABB me{tr.pos.x - tr.r, tr.pos.y - tr.r, tr.r * 2, tr.r * 2};
            auto collide = [&](const geometry::AABB &a)
            { return aabbOverlap(me, a); };

            bool hit = false;
            for (auto &aabb : map.colliders)
            {
                if (collide(aabb))
                {
                    hit = true;
                    break;
                }
            }
            if (hit)
            {
                // X 롤백 후 재확인
                tr.pos.x = prev.x;
                me.x = tr.pos.x - tr.r;
                bool hitX = false;
                for (auto &aabb : map.colliders)
                {
                    if (collide(aabb))
                    {
                        hitX = true;
                        break;
                    }
                }
                if (hitX)
                {
                    // 둘 다 막힘 → 완전 롤백
                    tr.pos = prev;
                    me.x = tr.pos.x - tr.r;
                    me.y = tr.pos.y - tr.r;
                }
                else
                {
                    // X 통과, Y 롤백
                    tr.pos.y = prev.y;
                    me.y = tr.pos.y - tr.r;
                }
            }

            // 카메라 추적 + 월드 경계 클램프
            float hw = cam.getSize().x * 0.5f;
            float hh = cam.getSize().y * 0.5f;
            float cx = movement::clampf(tr.pos.x, worldBounds.x + hw, worldBounds.x + worldBounds.w - hw);
            float cy = movement::clampf(tr.pos.y, worldBounds.y + hh, worldBounds.y + worldBounds.h - hh);
            cam.setCenter(cx, cy);
        }

        // ==== 렌더 ====
        win.setView(cam);
        win.clear(sf::Color(28, 30, 34));

        // 화면 내 타일 범위만 그리기 (컬링)
        sf::FloatRect view = cam.getViewport(); // viewport normalized, not what we want
        // 대신 화면 월드 bounds 계산
        sf::Vector2f cc = cam.getCenter();
        sf::Vector2f sz = cam.getSize();
        float viewL = cc.x - sz.x * 0.5f;
        float viewT = cc.y - sz.y * 0.5f;
        float viewR = cc.x + sz.x * 0.5f;
        float viewB = cc.y + sz.y * 0.5f;

        int minX = std::max(0, int(std::floor(viewL / TS)) - 1);
        int maxX = std::min(map.w - 1, int(std::ceil(viewR / TS)) + 1);
        int minY = std::max(0, int(std::floor(viewT / TS)) - 1);
        int maxY = std::min(map.h - 1, int(std::ceil(viewB / TS)) + 1);

        sf::RectangleShape tile({(float)TS, (float)TS});
        for (int y = minY; y <= maxY; ++y)
        {
            for (int x = minX; x <= maxX; ++x)
            {
                int v = map.tiles[y * map.w + x];
                tile.setPosition((float)(x * TS), (float)(y * TS));
                tile.setFillColor(v == 1 ? sf::Color(70, 75, 85) : sf::Color(46, 52, 64));
                win.draw(tile);
            }
        }

        // 플레이어
        sf::CircleShape pc(tr.r, 18);
        pc.setOrigin(tr.r, tr.r);
        pc.setPosition(tr.pos.x, tr.pos.y);
        pc.setFillColor(sf::Color(100, 200, 255));
        win.draw(pc);

        // 방향선
        sf::Vertex line[2] = {
            sf::Vertex(sf::Vector2f(tr.pos.x, tr.pos.y), sf::Color(200, 200, 200)),
            sf::Vertex(sf::Vector2f(tr.pos.x + (facingRight ? 18.f : -18.f), tr.pos.y), sf::Color(200, 200, 200))};
        win.draw(line, 2, sf::Lines);

        // HUD(맵 바)
        win.setView(win.getDefaultView());
        sf::RectangleShape bar({240.f, 10.f});
        bar.setPosition(16.f, 16.f);
        bar.setFillColor(sf::Color(90, 200, 120));
        win.draw(bar);

        win.display();
    }
    return 0;
}

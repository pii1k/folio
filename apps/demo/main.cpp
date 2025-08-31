#include "adapters/sfml/sfml_input.hpp"
#include "game/combat/fighter.hpp"
#include "game/core/time.hpp"
#include "game/geometry/types.hpp"
#include "game/movement/character_controller.hpp"
#include <SFML/Graphics.hpp>

using namespace folio;

int main()
{
    sf::RenderWindow win(sf::VideoMode(960, 540), "folio demo - 2D quarter-view PoC");
    win.setFramerateLimit(120);

    // 아레나 경계
    geometry::AABB arena{20, 20, 920, 500};

    // 플레이어
    geometry::Transform tr;
    tr.pos = {160.f, 270.f};
    tr.r = 14.f;
    movement::CharacterController ctrl;
    combat::Fighter fighter{};
    bool facingRight = true;

    // 입력
    adapters::SfmlInput input;
    bool attackPrev = false;
    std::vector<geometry::AABB> hitboxes; // 프레임 히트박스 표시용

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

            // 입력 샘플
            auto s = input.sample();

            // 마우스 기준 바라보는 방향
            facingRight = input.facingRight(win, tr.pos.x);

            // 이동/대시
            ctrl.tick(tr, s, FixedDelta{dt}, arena);

            // 공격(좌클릭 엣지): 히트박스 1회 생성
            if (input.attackPressedEdge(attackPrev))
            {
                auto box = combat::makeSlashBox(tr, facingRight, fighter.atk_range);
                hitboxes.push_back(box);
            }

            // 히트박스는 0.08초만 유지하도록 간단 삭제
            // (여기서는 시간 저장 안 했으니 1틱만 표시 후 제거)
        }
        hitboxes.clear(); // 단발 이펙트처럼 보이게 프레임마다 클리어

        // ---- 렌더 ----
        win.clear(sf::Color(28, 30, 34));

        // ground
        sf::RectangleShape ground({arena.w, arena.h});
        ground.setPosition(arena.x, arena.y);
        ground.setFillColor(sf::Color(46, 52, 64));
        win.draw(ground);

        // player (원)
        sf::CircleShape pc(tr.r, 18);
        pc.setOrigin(tr.r, tr.r);
        pc.setPosition(tr.pos.x, tr.pos.y);
        pc.setFillColor(sf::Color(100, 200, 255));
        win.draw(pc);

        // facing indicator
        sf::Vertex line[2] = {
            sf::Vertex(sf::Vector2f(tr.pos.x, tr.pos.y), sf::Color(200, 200, 200)),
            sf::Vertex(sf::Vector2f(tr.pos.x + (facingRight ? 18.f : -18.f), tr.pos.y), sf::Color(200, 200, 200))};
        win.draw(line, 2, sf::Lines);

        // attack hitbox (프레임 단발 표시)
        for (auto &hb : hitboxes)
        {
            sf::RectangleShape r({hb.w, hb.h});
            r.setPosition(hb.x, hb.y);
            r.setFillColor(sf::Color(220, 90, 90, 120));
            win.draw(r);
        }

        // stamina bar (상단 좌측)
        float stamina = ctrl.runtime().stamina;
        sf::RectangleShape sbg({120.f, 8.f});
        sbg.setFillColor(sf::Color(20, 20, 20));
        sf::RectangleShape sfg({1.2f * stamina, 8.f});
        sfg.setFillColor(sf::Color(90, 220, 120));
        sbg.setPosition(24, 24);
        sfg.setPosition(24, 24);
        win.draw(sbg);
        win.draw(sfg);

        win.display();
    }
    return 0;
}

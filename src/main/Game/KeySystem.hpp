#pragma once

#include "Constants.hpp"
#include "System.hpp"
#include "Utils.hpp"
#include "events/AttackEvent.hpp"
#include "events/CollectEvent.hpp"
#include "events/NumEvent.hpp"
#include "events/RemoveFromInventoryEvent.hpp"
#include <chrono>
#include <iostream>
#include <unordered_map>

#define NUMEVENT(X)                                                                                                    \
    bool num##X = sf::Keyboard::isKeyPressed(sf::Keyboard::Num##X);                                                    \
    if (num##X) {                                                                                                      \
        em.update<PlayerComponent>([&](auto& ent, PlayerComponent& player) {                                           \
            if (time_to_click[ent.get_id()] < t) {                                                                     \
                evm.notify(NumXEvent(ent.get_id(), X));                                                                \
                time_to_click[ent.get_id()] = t + 100_ms;                                                              \
            }                                                                                                          \
        });                                                                                                            \
    }

#define NUMSAVEINVEVENT(X)                                                                                             \
    bool numpad##X = sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad##X);                                              \
    if (numpad##X) {                                                                                                   \
        em.update<PlayerComponent>([&](auto& ent, PlayerComponent& player) {                                           \
            if (time_to_click[ent.get_id()] < t) {                                                                     \
                evm.notify(SetValueEventInventoryEvent(ent.get_id(), X + 1));                                          \
                time_to_click[ent.get_id()] = t + 100_ms;                                                              \
            }                                                                                                          \
        });                                                                                                            \
    }

class KeySystem : public SystemHandle, public SystemInterface {
private:
    std::unordered_map<ECS::EntityId, sf::Time> time_to_click;

public:
    void update(EventManager& evm, EntityManager& em, SystemManager&, sf::Time t) override {

        bool left = sf::Keyboard::isKeyPressed(LEFT);
        bool right = sf::Keyboard::isKeyPressed(RIGHT);
        bool up = sf::Keyboard::isKeyPressed(UP);
        bool down = sf::Keyboard::isKeyPressed(DOWN);

        bool collect = sf::Keyboard::isKeyPressed(COLLECT);
        bool attack = sf::Mouse::isButtonPressed(ATTACK);

        bool remove_item = sf::Keyboard::isKeyPressed(REMOVE_ITEM);

        if (left || right || up || down) {
            em.update<PlayerComponent, MoveComponent, SpriteComponent>(
                [&](auto& entity, PlayerComponent& pl, MoveComponent& mv, SpriteComponent& sprite) {
                    auto tmpx = mv.data.x;
                    auto tmpy = mv.data.y;
                    mv.data.x = [=, rs = t.asMilliseconds()](double tm) {
                        double alpha = sigmoid(tm, 0.1, rs);
                        return OPRTIMIZE_MULT_ZERO(alpha, 6 * (right - left) * std::exp((rs - tm) / 50.0)) +
                               OPRTIMIZE_MULT_ZERO((1 - alpha), tmpx(tm));
                    };
                    mv.data.y = [=, rs = t.asMilliseconds()](double tm) {
                        double alpha = sigmoid(tm, 0.1, rs);
                        return OPRTIMIZE_MULT_ZERO(alpha, 6 * (down - up) * std::exp((rs - tm) / 50.0)) +
                               OPRTIMIZE_MULT_ZERO((1 - alpha), tmpy(tm));
                    };
                });
        }

        if (attack) {
            em.update<PlayerComponent>([&](auto& ent, PlayerComponent& player) {
                if (time_to_click[ent.get_id()] < t) {
                    evm.notify(AttackEvent(ent.get_id()));
                    time_to_click[ent.get_id()] = t + 100_ms;
                }
            });
        }

        if (collect) {
            em.update<PlayerComponent>([&](auto& ent, PlayerComponent& player) {
                if (time_to_click[ent.get_id()] < t) {
                    evm.notify(CollectEvent(ent.get_id()));
                    time_to_click[ent.get_id()] = t + 100_ms;
                }
            });
        }

        NUMEVENT(0)
        NUMEVENT(1)
        NUMEVENT(2)
        NUMEVENT(3)
        NUMEVENT(4)
        NUMEVENT(5)
        NUMEVENT(6)
        NUMEVENT(7)

        if (remove_item) {
            em.update<PlayerComponent>([&](auto& ent, PlayerComponent& player) {
                if (time_to_click[ent.get_id()] < t) {
                    evm.notify(RemoveFromInventoryEvent(ent.get_id()));
                    time_to_click[ent.get_id()] = t + 100_ms;
                }
            });
        }

        NUMSAVEINVEVENT(0)
        NUMSAVEINVEVENT(1)
        NUMSAVEINVEVENT(2)
        NUMSAVEINVEVENT(3)
    }
};

#undef NUMEVENT
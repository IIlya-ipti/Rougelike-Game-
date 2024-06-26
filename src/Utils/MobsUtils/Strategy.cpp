#include "Component.hpp"
#include "EntitiesList.hpp"
#include "EntityManager.hpp"
#include "MobsUtils/Strategy.hpp"
#include "Vec2.hpp"

using namespace ECS;

/////////////////////////
// Aggressive Strategy //
/////////////////////////

void AggressiveStrategy::update_coord(ECS::EntityManager& em, ECS::EventManager& evm, ECS::EntityId eid, sf::Time t,
                                      StrategyContext* context) {
    Vec2 vector_between = {0, 0};
    auto& pos_ent = em.template get_component<PositionComponent>(eid);
    auto& sprite_ent = em.template get_component<SpriteComponent>(eid);
    auto center_pos = ECS::center_of_mass(sprite_ent.data.sprite, pos_ent.data);
    em.update<PlayerComponent, SpriteComponent, PositionComponent>(
        [&](auto& ent, PlayerComponent&, SpriteComponent& player_sprite, PositionComponent const& player_pos) {
            auto player_center_pos = center_of_mass(player_sprite.data.sprite, player_pos.data);
            if (center_pos.dist(player_center_pos) < 200) {
                vector_between += center_pos - player_center_pos;
            }
        });
    vector_between.normalize();
    auto& mv = em.template get_component<MoveComponent>(eid);

    if (vector_between != Vec2(0, 0)) {
        mv.data.directions_t_clean[0] = [vector_between, rs = t.asMilliseconds() / 1000.0](double tm) -> Vec2 {
            tm /= 1000;
            double alpha = sigmoid(tm, 3, rs);
            return Vec2{(1 - alpha) * 3 * -vector_between.x_ * std::exp((rs - tm) / 50.0),
                        (1 - alpha) * 3 * -vector_between.y_ * std::exp((rs - tm) / 50.0)};
        };
    } else if (typeid(*(context->get_default_strategy())) == typeid(PatrolStrategy)) {
        context->set_strategy(context->get_default_strategy());
    }
}

void AggressiveStrategy::update_and_check_state(ECS::EntityManager& em, ECS::EventManager& evm, ECS::EntityId eid,
                                                sf::Time t, StrategyContext* sc) {
    em.update_by_id<HealthComponent>(eid, [&](auto& eid, HealthComponent& hc) {
        if (hc.data.hp <= MOB_MIN_ATTACK_HEALTH_TO_COWARD) {
            std::shared_ptr<Strategy> ptr = std::make_shared<CowardStrategy>();
            sc->set_strategy(ptr);
        }
    });
}

void AggressiveStrategy::attack(ECS::EntityManager& em, ECS::EventManager& evm, ECS::EntityId eid, sf::Time t,
                                StrategyContext* context) {
    auto& pos_ent = em.template get_component<PositionComponent>(eid);
    auto& sprite_ent = em.template get_component<SpriteComponent>(eid);
    auto center_pos = ECS::center_of_mass(sprite_ent.data.sprite, pos_ent.data);
    bool is_attck = false;
    em.update<PlayerComponent, SpriteComponent, PositionComponent, HealthComponent, MoveComponent>(
        [&](auto& ent, PlayerComponent&, SpriteComponent& player_sprite, PositionComponent const& player_pos,
            HealthComponent& player_hc, MoveComponent& player_mc) {
            auto player_center_pos = center_of_mass(player_sprite.data.sprite, player_pos.data);
            if (center_pos.dist(player_center_pos) < 40) {
                is_attck = true;
            }
        });
    if (is_attck) {
        evm.notify(AttackEvent(eid));
    }
}

/////////////////////
// Coward Strategy //
/////////////////////

void CowardStrategy::update_coord(ECS::EntityManager& em, ECS::EventManager& evm, ECS::EntityId eid, sf::Time t,
                                  StrategyContext* context) {
    Vec2 vector_between = {0, 0};
    auto& pos_ent = em.template get_component<PositionComponent>(eid);
    auto& sprite_ent = em.template get_component<SpriteComponent>(eid);
    auto center_pos = ECS::center_of_mass(sprite_ent.data.sprite, pos_ent.data);
    em.update<PlayerComponent, SpriteComponent, PositionComponent>(
        [&](auto& ent, PlayerComponent&, SpriteComponent& player_sprite, PositionComponent const& player_pos) {
            auto player_center_pos = center_of_mass(player_sprite.data.sprite, player_pos.data);
            if (center_pos.dist(player_center_pos) < 200) {
                vector_between += center_pos - player_center_pos;
            }
        });
    vector_between.normalize();
    auto& mv = em.template get_component<MoveComponent>(eid);

    if (vector_between != Vec2{0, 0}) {
        mv.data.directions_t_clean[0] = [vector_between, rs = t.asMilliseconds() / 1000.0](double tm) {
            tm /= 1000;
            double alpha = sigmoid(tm, 3, rs);
            return Vec2{
                (1 - alpha) * (rand() % 10 - 5) + (1 - alpha) * 5 * vector_between.x_ * std::exp((rs - tm) / 50.0),
                (1 - alpha) * (rand() % 10 - 5) + (1 - alpha) * 5 * vector_between.y_ * std::exp((rs - tm) / 50.0)};
        };
    } else if (typeid(*(context->get_default_strategy())) == typeid(PatrolStrategy)) {
        context->set_strategy(context->get_default_strategy());
    }
}

void CowardStrategy::update_and_check_state(ECS::EntityManager& em, ECS::EventManager& evm, ECS::EntityId eid,
                                            sf::Time t, StrategyContext* sc) {
    if (sc->get_default_strategy() == nullptr) {
        return;
    }
    if (typeid(*(sc->get_default_strategy())) == typeid(AggressiveStrategy)) {
        em.update_by_id<HealthComponent>(eid, [&](auto& ent, HealthComponent& hc) {
            if (hc.data.hp >= MOB_MAX_ATTACK_HEALTH_TO_AGGRESSIVE) {
                sc->set_strategy(sc->get_default_strategy());
            }
        });
    }
}

/////////////////////
// Patrol Strategy //
/////////////////////

void PatrolStrategy::update_coord(ECS::EntityManager& em, ECS::EventManager&, ECS::EntityId eid, sf::Time,
                                  StrategyContext* context) {
    Vec2 target_pos = get_point(eid);
    bool closer_player = false;
    em.update_by_id<SpriteComponent, PositionComponent, MoveComponent>(
        eid, [this, &em, &target_pos, &closer_player](auto& ent, SpriteComponent& sc, PositionComponent& pc,
                                                      MoveComponent& mc) {
            auto center_pos = ECS::center_of_mass(sc.data.sprite, pc.data);
            Vec2 diff = target_pos - center_pos;
            if (diff.get_norm() < 10) {
                get_next_id(ent.get_id());
            }
            if (diff.get_norm() > 4) {
                diff.normalize();
                diff *= 1.5;
            }
            em.update<PlayerComponent, SpriteComponent, PositionComponent>([&](auto& ent, PlayerComponent const&,
                                                                               SpriteComponent const& player_sprite,
                                                                               PositionComponent& player_pos) {
                auto player_center_pos = center_of_mass(player_sprite.data.sprite, player_pos.data);
                if (center_pos.dist(player_center_pos) < 200) {
                    closer_player = true;
                }
            });

            mc.data.directions_t_clean[0] = [diff](double t) { return diff; };
        });

    if (closer_player) {
        context->set_strategy(std::make_shared<AggressiveStrategy>());
    }
}

template <is_subscript Subscript_type>
std::size_t PatrolStrategy::get_id_closest_point(Subscript_type&& input_vec, Vec2 point) {
    std::size_t best_id = 0;
    double min_distance = std::numeric_limits<double>::max();
    Vec2 result = input_vec.at(0);
    for (std::size_t id = 0; id < input_vec.size(); id++) {
        double dist_between = input_vec[id].dist(point);
        if (dist_between < min_distance) {
            min_distance = dist_between;
            best_id = id;
        }
        result = input_vec[id];
    }
    return best_id;
}

std::size_t PatrolStrategy::get_next_id(ECS::EntityId eid) {
    std::size_t id = id_next_point[eid];
    std::size_t num_points = id_trajectory[eid].size();
    id = (id + 1) % num_points;
    id_next_point[eid] = id;
    return id;
}

Vec2 PatrolStrategy::get_point(ECS::EntityId eid) {
    std::size_t id = id_next_point[eid];
    return id_trajectory[eid][id];
}

void PatrolStrategy::set_points(ECS::EntityId eid, Vec2 actual_pos, std::vector<Vec2> vec) {
    if (vec.empty()) {
        throw std::runtime_error("vector size > 0 !");
    }
    std::size_t closest_id = get_id_closest_point(vec, actual_pos);
    id_trajectory[eid] = std::move(vec);
    id_next_point[eid] = closest_id;
}
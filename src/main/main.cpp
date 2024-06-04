#include <iostream>
#include "Engine.hpp"
#include <type_traits>
#include "Component.hpp"
#include <EventManager.hpp>
#include "System.hpp"
#include "SystemManager.hpp"
#include <chrono>
#include <thread>
#include <SFML/Graphics.hpp>

namespace ECS
{
    struct collisionEvent : public Event<ECS::collisionEvent> {
        collisionEvent(int x, int y) : x_{x}, y_{y} {} 
        int x_;
        int y_;
    };
}

using namespace ECS;


class CollisionSystem2 : public SystemHandle, public SystemInterface {
public:
    void update(EventManager&, EntityManager&, SystemManager&) {}
};

class PrintSystem : public SystemHandle, public SystemInterface {
public:
    void update(EventManager&, EntityManager& em, SystemManager&) {
        std::cout << "--------------------------" << std::endl;
        em.update<PositionComponent>([](auto&, PositionComponent const& comp)
        {
            std::cout << comp.data.x << " " << comp.data.y << std::endl;
        }
        );
    }
};

int main() {

    sf::RenderWindow window(sf::VideoMode(200,200), "Hello From SFML");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Magenta);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed){
                window.close();
            }
        }
        window.clear();
        window.draw(shape);
        window.display();
    }
    return 0;

    // ECS::SystemManager sm;
    // sm.add_system(std::make_unique<GeneratorSystem>());
    // sm.add_system(std::make_unique<MoveSystem>());
    // sm.add_system(std::make_unique<PrintSystem>());
    // while (true)
    // {
    //   std::chrono::milliseconds timespan(500); // or whatever
    //   std::this_thread::sleep_for(timespan);
    //   sm.update();
    // }
    
    // EventManager em;
    // collisionEvent ce{1,2};
    // // auto system = std::make_shared<CollisionSystem>();

    // em.subscribe<collisionEvent>(system);
    // em.notify(ce);

    // EntityManager em;
    // auto val1 = em.allocEntity<DogEntity>();
    // auto val2 = em.allocEntity<DogEntity>();
    // auto val3 = em.allocEntity<DogEntity>();
    // auto val4 = em.allocEntity<DogEntity>();
    // auto val5 = em.allocEntity<NpcEntity>();

    // std::cout << val4->get_component<PositionComponent>().data.x << std::endl;
    // std::cout << val4->get_component<MoveComponent>().data.x << std::endl;
    // std::cout << val5->get_component<MoveComponent>().data.x << std::endl;
    // val4->get_component<PositionComponent>().data.x += 1;
    // val4->get_component<PositionComponent>().data.y += 1;

    // em.update<PositionComponent>([](auto& ent, PositionComponent &vl){
    //     std::cout << ent.get_id() << " <<id---------------" << std::endl;
    //     std::cout << vl.data.x << std::endl;
    //     std::cout << vl.data.y << std::endl;
    // });

    // em.update<PositionComponent, MoveComponent>([](auto& ent, PositionComponent &vl, MoveComponent& mv){
    //     std::cout << ent.get_id() << " <<id---------------" << std::endl;
    //     std::cout << vl.data.x << std::endl;
    //     std::cout << vl.data.y << std::endl;
    //     std::cout << mv.data.x << std::endl;
    //     std::cout << mv.data.y << std::endl;
    // });

    // std::vector<EntityId> ents;

    // em.update<PositionComponent>([&](auto& ent, PositionComponent& comp)
    // {
    //     ents.push_back(ent.get_id());
    // });

    // for(auto& vl : ents)
    // {
    //     em.removeEntity(vl);
    // }

    // ents.clear();
    // em.update<>([&](auto& ent){
    //     std::cout << ent.get_id() << std::endl;
    //     ents.push_back(ent.get_id());
    // });

    // for(auto& vl : ents)
    // {
    //     em.removeEntity(vl);
    // }
    
    // std::cout << "id before" << std::endl;
    // em.update<>([](auto& ent){
    //     std::cout << ent.get_id() << std::endl;
    // });




    // // std::cout << "-----------" <<std::endl;
    // // em.update<MoveComponent>([](auto& ent, MoveComponent &vl){
    // //     std::cout << ent.get_id() << " " << vl.data.x << std::endl;
    // //     std::cout << ent.get_id() << " " << vl.data.x << std::endl;
    // // });
    // // std::cout << "-----------" <<std::endl;

    return 0;
}

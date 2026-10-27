#include "ecs/ecs.hpp"

namespace ecs{
    void test(){
        EntityManager<> e{100};

        auto e1 = e.create_entity();

        auto e2 = e.create_entity();

        auto e3 = e.create_entity();

        auto e4 = e.create_entity();

        //e.print();

        e.destroy_entity(e4);

        e.set_signature(e3, 0xa9bfffff);

        //e.print();

        e4 = e.create_entity();
        
        //e.print();

        e.destroy_entity(e3);

        e3 = e.create_entity();

        ComponentManager c{32};

        struct Position{
            float x = 0, y = 0, z = 0;
        };

        c.register_component<Position>();
        c.register_component<float>();
        c.register_component<std::string>();

        c.add_component_to_entity<Position>(e3);
        c.add_component_to_entity<Position>(e2);
        c.add_component_to_entity<float>(e2);
        c.add_component_to_entity<float>(e3);
        c.add_component_to_entity<std::string>(e3);

        c.set_component<Position>(e3, {10.f,10.f,10.f});

        c.print();

        BaseID<int, 0> t;
        std::cout << t() << std::endl;
    }
}
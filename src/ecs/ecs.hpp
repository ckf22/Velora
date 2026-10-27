#pragma once

#include "ecs/component-array.hpp"
#include "ecs/component-manager.hpp"
#include "ecs/entity-manager.hpp"
#include "ecs/types.hpp"

namespace ecs{

template<Component _MAX_COMPONENTS = 32>
class ECS{
  public:
  private:
    std::unique_ptr<EntityManager<_MAX_COMPONENTS>> entity_manager;
    std::unique_ptr<ComponentManager> component_manager;
};

}
#include "ecs/system.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

namespace ecs{


class SystemManager{
    SystemManager(){}

    template<typename T>
    void add_system(std::shared_ptr<T> system){

    }

    std::unordered_map<SystemID, std::size_t> system_to_index;
    std::unordered_map<std::size_t, SystemID> index_to_system;

    std::vector<std::shared_ptr<ISystem>> systems;
};

}
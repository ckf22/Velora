#pragma once

#include "ecs/component-array.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <limits>

namespace ecs{

class ComponentManager{
    struct TypeData{
        std::string name;
        Component component_id{};
        // Required for void ptr access
        std::size_t byte_size;
        std::size_t index;

        void print() const noexcept {
            std::cout << "{name:'" << this->name << "',component_id:" << this->component_id << ",size(bytes):" << this->byte_size << ",index:" << this->index << "}"; 
        }
    };
  public:
    ComponentManager(const std::size_t max_components){
        this->arrays.resize(max_components);
    }
    ~ComponentManager(){}

    template<typename T>
    void register_component(){
        assert( this->name_map.find(typeid(T).name()) == this->name_map.end() && "Type must not be registered more than once" );

        TypeData info{
            .name = typeid(T).name(),
            .component_id = this->generate_next_id(),
            .byte_size = sizeof(T),
            .index = this->array_count
        };

        this->name_map.insert({info.name, info});
        this->component_id_map.insert({info.component_id, info});

        this->arrays[info.index] = std::make_shared<ComponentArray<T>>();

        this->array_count++;
    }


    template<typename T>
    inline void remove_component(Entity entity){
        this->get_component_array<T>()->entity_destroyed();
    }

    template<typename T>
    [[nodiscard]] inline T& get_component(Entity entity) const {
        return this->get_component_array<T>()->get_component(entity);
    }

    template<typename T>
    inline void set_component(Entity entity, T component){
        this->get_component_array<T>()->insert_data(entity, component);
    }

    template<typename T>
    inline void add_component_to_entity(Entity entity){
        this->get_component_array<T>()->add_entity(entity);
    }

    void destroy_entity(Entity entity){
        for(auto& array : this->arrays){
            array->entity_destroyed(entity);
        }
    }

    template<typename T>
    [[nodiscard]] inline std::shared_ptr<ComponentArray<T>> get_component_array() const noexcept {
        auto it = name_map.find(typeid(T).name());

        assert( it != this->name_map.end() && "Component type must be registered before usage" );

        return std::static_pointer_cast<ComponentArray<T>>(this->arrays[it->second.index]);
    }

    void print(){
        for(const auto& it : name_map){
            it.second.print();
            std::cout << std::endl;
            this->arrays[it.second.index]->print();
            std::cout << std::endl;
        }
    }
  private:
    [[nodiscard]] Component generate_next_id() noexcept {
        Component i = this->array_count;
        for(; i < std::numeric_limits<Component>::max(); ++i){
            if(this->used_ids.find(i) == this->used_ids.end()){
                break;
            }
        }

        this->used_ids.insert(i);

        return i;
    }

    std::unordered_set<Component> used_ids;

    std::unordered_map<Component, TypeData> component_id_map;
    std::unordered_map<std::string, TypeData> name_map;

    std::vector<std::shared_ptr<IComponentArray>> arrays;

    std::size_t array_count = 0;
};

}
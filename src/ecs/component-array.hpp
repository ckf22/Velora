#pragma once

#include "ecs/types.hpp"

#include <vector>
#include <unordered_map>
#include <list>
#include <cassert>
#include <iostream>

namespace ecs{

class IComponentArray{
  protected:
    virtual ~IComponentArray() = default;
  public:
    // TEMP
    virtual void print() = 0;
    // To be called when Entity is destroyed
    virtual void entity_destroyed(Entity entity) = 0;
};

template<typename T, std::size_t block_size = 16384, bool _EXCEPTIONS = false>
class ComponentArray : public IComponentArray{
  public:
    ComponentArray(){
        // temp
        this->data.resize(100);
    }

    void entity_destroyed(Entity entity){
        auto it = this->entity_to_index.find(entity);
        if( it != this->entity_to_index.end() ){
            this->remove_data(entity);
        }
    }

    void remove_data(Entity entity){
        auto it = this->entity_to_index.find(entity);

        if constexpr (_EXCEPTIONS)
            if(it == this->entity_to_index.end())
                throw std::logic_error("Entity does not exist");
        if constexpr (!_EXCEPTIONS)
            assert( it != this->entity_to_index.end() && "Entity does not exist" );

        std::size_t old_index = it->second;

        if( old_index != this->element_count - 1 ){
            this->data[old_index] = this->data[this->element_count-1];

            auto it2 = this->index_to_entity.find(this->element_count-1);

            this->index_to_entity[old_index] = it->second;

            this->entity_to_index[it2->second] = old_index;
            
            this->index_to_entity.erase(it2);
        }
        else{
            this->index_to_entity.erase(it->second);
        }

        this->entity_to_index.erase(entity);

        this->element_count--;
    }

    inline void add_entity(Entity entity){
        assert( this->entity_to_index.find(entity) == this->entity_to_index.end() && "Entity is already registered " );

        this->entity_to_index.insert({entity, this->element_count});
        this->index_to_entity.insert({this->element_count, entity});

        this->element_count++;
    }

    inline void insert_data(Entity entity, T value) noexcept {
        auto it = this->entity_to_index.find(entity);

        assert( it != this->entity_to_index.end() && "Component for this Entity does not exist" );

        this->data[it->second] = value;
    }

    [[nodiscard]] inline T& get_component(Entity entity){
        auto it = this->entity_to_index.find(entity);

        assert( it != this->entity_to_index.end() && "Component for this Entity does not exist" );

        return this->data[it->second];
    }
    void print(){
        std::cout << "Entity | Index" << std::endl;
        for(auto& it : this->entity_to_index){
            std::cout << it.first << ':' << it.second << "    " << std::endl;
        }
        std::cout << "Index | Entity" << std::endl;
        for(auto& it : this->index_to_entity){
            std::cout << it.first << ':' << it.second << std::endl;
        }
        std::cout << std::endl;
    }
  private:
    std::unordered_map<Entity, std::size_t> entity_to_index;
    std::unordered_map<std::size_t, Entity> index_to_entity;

    std::vector<T> data;

    std::size_t element_count;
};

}
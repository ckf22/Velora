#pragma once

#include "ecs/types.hpp"

#include <bitset>
#include <queue>
#include <unordered_map>

#include <cassert>

#include <iostream>

namespace ecs{

template<Component _MAX_COMPONENTS = 32, bool _ESCEPTIONS = false>
class EntityManager{
    using Signature = std::bitset<_MAX_COMPONENTS>;
  public:
    EntityManager(u_int32_t expected_entity_count){

        this->signatures.resize(expected_entity_count);

        for(u_int32_t i = 0; i < expected_entity_count; ++i){
            this->avalible_ids.push(Entity{i});
        }

    }

    ~EntityManager(){}


    [[nodiscard]] Entity create_entity() noexcept {
        assert( this->living_entity_count+1 < this->signatures.size() );

        auto id = this->generate_component_id();

        this->entity_to_index.insert({id, this->living_entity_count});
        this->index_to_entity.insert({this->living_entity_count, id});

        this->living_entity_count++;

        return id;
    }

    void destroy_entity(Entity entity) noexcept {
        // it->first == entity and it->second is the index
        auto it = this->entity_to_index.find(entity);

        assert( it != this->entity_to_index.end() && "Entity does not exist" );

        // the signature at this index is to be freed, and may be repopulated by the last signatures to ensure memory efficiency
        std::size_t old_index = it->second;

        // triggers when the signature of the entity to be freed does not occupy last slot of the RAM block
        if( old_index != this->living_entity_count - 1 ){
            this->signatures[old_index] = this->signatures[this->living_entity_count-1];
            this->signatures[this->living_entity_count-1] = Signature{};

            auto it2 = this->index_to_entity.find(this->living_entity_count-1);

            this->index_to_entity[old_index] = it2->second;

            this->entity_to_index[it2->second] = old_index;
            this->index_to_entity.erase(this->living_entity_count-1);

        }
        else{ // If it was the last signature the index will be deleted
            this->index_to_entity.erase(old_index);
        }

        // Deleting old 
        this->entity_to_index.erase(it);

        this->living_entity_count--;

        // temp
        this->avalible_ids.push(entity);
    }

    [[nodiscard]] Signature& get_signature(Entity entity) const noexcept {
        auto it = this->entity_to_index.find(entity);

        assert( it != this->entity_to_index.end() && "Entity does not exist" );

        return this->signatures[it->second];
    }

    void set_signature(Entity entity, Signature signature) noexcept {
        auto it = this->entity_to_index.find(entity);

        assert( it != this->entity_to_index.end() && "Entity does not exist" );

        this->signatures[it->second] = signature;
    }

    [[nodiscard]] bool does_entity_exist(Entity entity) const noexcept {
        return this->entity_to_index.find(entity) != this->entity_to_index.end();
    }

    void print(){
        std::cout << "Entity | Index" << std::endl;
        for(auto& it : this->entity_to_index){
            std::cout << it.first << ':' << it.second << "    " << this->signatures[it.first] << std::endl;
        }
        std::cout << "Index | Entity" << std::endl;
        for(auto& it : this->index_to_entity){
            std::cout << it.first << ':' << it.second << std::endl;
        }
        std::cout << std::endl;
    }
  private:

    [[nodiscard]] Entity generate_component_id() noexcept {
        assert( this->avalible_ids.size() > 0 && "out of component ids" );

        Entity ret = this->avalible_ids.front();

        this->avalible_ids.pop();

        return ret;
    }

    std::queue<Entity> avalible_ids{};

    // Also used to indicate next vector writing index
    uint32_t living_entity_count = 0;

    std::unordered_map<Entity, std::size_t> entity_to_index;
    std::unordered_map<std::size_t, Entity> index_to_entity;

    std::vector<Signature> signatures;
    
};

} // namespace ecs

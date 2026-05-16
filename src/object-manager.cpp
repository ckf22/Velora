#include "object-manager.hpp"

#include <iostream>
#include <string.h>
#include <memory>

namespace velora{

ObjectManager::ObjectManager(){
}
ObjectManager::~ObjectManager(){
}

u_int32_t ObjectManager::upload_shader_data(void * dest, int max_bytes){
    int running_sum = max_bytes;
    for(auto& it : this->objects){
        if(running_sum - it.get_required_ram() < 0) break;

        auto buffer = it.write_data(dest);
        running_sum -= buffer;
        dest += buffer;
    }

    return max_bytes - running_sum;
}

void ObjectManager::add_object(Object& _object){
    this->vertex_count += _object.get_vertex_count();
    this->max_vertex_count += _object.get_max_vertex_count();

    this->objects.push_back(_object);
}


} // namespace velora
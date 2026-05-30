#include "object-manager.hpp"

#include <iostream>
#include <string.h>
#include <memory>

namespace velora{

ObjectManager::ObjectManager(){
}
ObjectManager::~ObjectManager(){
}

void ObjectManager::add_draw_calls(VkCommandBuffer& cmd_buffer){
    int index_begin = 0, index_offset = 0;
    for(auto& it : this->objects){
        vkCmdDrawIndexed(cmd_buffer, it.get_index_count(), 1, index_begin, index_offset, 0);
        index_offset += it.get_vertex_count();
        index_begin += it.get_index_count();
    }
}

u_int32_t ObjectManager::upload_shader_data(void * vertex_dest, void * index_dest, int max_vertex_bytes, int max_index_bytes){
    int running_sum_vertex = max_vertex_bytes, running_sum_index = max_index_bytes;
    for(auto& it : this->objects){
        // Copying vertex data
        if(running_sum_vertex - it.get_required_ram() < 0) break;
        auto buffer = it.write_vertex_data(vertex_dest);
        running_sum_vertex -= buffer;
        vertex_dest += buffer;

        // Copying indices
        if(running_sum_index - (it.get_index_count() * sizeof(u_int32_t)) < 0) break;
        buffer = it.write_index_data(index_dest);
        running_sum_index -= buffer;
        index_dest += buffer;
    }

    return max_vertex_bytes - running_sum_vertex + max_index_bytes - running_sum_index;
}

void ObjectManager::add_object(Object& _object){
    this->vertex_count += _object.get_vertex_count();
    this->max_vertex_count += _object.get_max_vertex_count();

    this->objects.push_back(_object);
}


} // namespace velora
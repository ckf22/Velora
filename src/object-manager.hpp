#pragma once

#include "object.hpp"

#include <vector>
#include <list>
#include <chrono>

namespace velora{

class ObjectManager{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
  public:
    ObjectManager();
    ~ObjectManager();

    ObjectManager& operator=(ObjectManager&) = delete;
    ObjectManager(ObjectManager&) = delete;

    u_int32_t get_vertex_count(){ return vertex_count; }
    u_int32_t get_required_ram(){ return vertex_count * sizeof(Vertex); }

    void add_object(Object& _object);
    u_int32_t upload_shader_data(void * vertex_dest, void * index_dest, int max_vertex_bytes = -1, int max_index_bytes = -1);
    u_int32_t upload_transforms(void * dest, u_int32_t max_bytes = -1);
    void add_draw_calls(VkCommandBuffer& cmd_buffer);
  private:
    std::list<Object> objects;
    u_int32_t vertex_count = 0;
    u_int32_t max_vertex_count = 0;
};

}
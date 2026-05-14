#pragma once

#include "device.hpp"
#include "object.hpp"

#include <vector>
#include <chrono>

namespace velora{

class ObjectManager{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
  public:
    struct ConstantRanges{
      alignas(16) glm::mat4 projection{1};
      alignas(16) glm::vec4 light_color{}; // color, then intensity
      glm::vec3 light{}; // first three values represent direction
      glm::vec3 ambient_light{};
    };

    ObjectManager(Device& _device, const u_int32_t _frame_count, std::vector<Vertex> _vertices);
    ~ObjectManager();

    ObjectManager& operator=(ObjectManager&) = delete;
    ObjectManager(ObjectManager&) = delete;

    u_int32_t get_vertex_count(){ return vertex_count; }

    void update_shader_data(std::chrono::microseconds dt, u_int32_t target_index);
    void bind_vertex_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index);
    void push_constant_ranges(VkCommandBuffer& cmd_buffer, VkPipelineLayout& layout);
  private:
    void create_buffer();

    const u_int32_t frame_count;
    Device& device;

    std::vector<VkBuffer> vertex_buffers;
    std::vector<VkDeviceMemory> vertex_buffer_memory;

    std::vector<Vertex> vertices;
    //std::vector<Object> objects;
    u_int32_t vertex_count = 0;

    float rotation_z = 0;
    TransformComponent tranform;
};

}
#pragma once

#include "object-manager.hpp"
#include "camera.hpp"
#include "device.hpp"

#include <memory>

namespace velora{

class RenderSystem{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
  public:
    struct PushConstantRange{
        glm::mat4 perspective_projection{1.f};
        glm::mat4 worldspace_transformation{1.f};
    };

    RenderSystem(Device& _device, const u_int32_t _frame_count);
    ~RenderSystem();

    RenderSystem& operator=(RenderSystem&) = delete;
    RenderSystem(RenderSystem&) = delete;

    u_int32_t get_vertex_count() { return objects.get_vertex_count(); }

    void update_shader_data();
    void upload_shader_data(u_int32_t frame_index);
    void push_constant_ranges(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout);
    void bind(VkCommandBuffer& cmd_buffer, u_int32_t frame_index);
  private:
    void populate();
    void create_buffers();

    u_int32_t frame_count;
    Device& device;

    std::vector<VkBuffer> vertex_buffers;
    std::vector<VkDeviceMemory> vertex_buffer_memory;
    u_int32_t buffer_size;

    Camera camera{};
    ObjectManager objects{};

    // temp
    TransformComponent transform{{0,0,.5f}, {1,1,1}, {.1f,.25f, .4f}, {0,0,.5f}};
};

} // namespace velora
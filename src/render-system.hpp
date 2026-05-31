#pragma once

#include "object-manager.hpp"
#include "movement-controller.hpp"
#include "descriptors.hpp"
#include "camera.hpp"
#include "device.hpp"
#include "buffer.hpp"

#include <memory>
#include <chrono>

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
    };

    RenderSystem(Device& _device, MovementController& _movement_controller, DescriptorManager& _descriptor_manager, 
      const u_int32_t _frame_count, int width, int height);
    ~RenderSystem();

    RenderSystem& operator=(RenderSystem&) = delete;
    RenderSystem(RenderSystem&) = delete;

    u_int32_t get_vertex_count() { return objects.get_vertex_count(); }

    void allocate_from_descriptor_set();
    void register_resize(int width, int height);

    void update_shader_data(std::chrono::microseconds dt);
    void upload_shader_data(u_int32_t frame_index);
    void fill_ssbo(VkCommandBuffer& cmd_buffer, u_int32_t frame_index); // copy command has to be executed through a command buffer
    void fill_command_buffer(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout, u_int32_t frame_index);
  private:
    void push_constant_ranges(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout);
    void populate();
    void create_buffer_objects();

    u_int32_t frame_count;
    int first_descriptor_id, layout_id;
    Device& device;
    MovementController& movement_controller;
    DescriptorManager& descriptor_manager;

    std::vector<std::unique_ptr<MyBuffer>> vertex_buffers;
    std::vector<std::unique_ptr<MyBuffer>> index_buffers;

    std::vector<std::unique_ptr<MyBuffer>> ssbo;
    std::vector<std::unique_ptr<MyBuffer>> ssbo_staging;

    float aspect_ratio;
    Camera camera{};
    ObjectManager objects{};

    // temp
    TransformComponent transform{{0,0,0.f}, {2,2,2}, {.0f,.0f, .0f}, {0,0,.0f}};
    float dy = 0;
};

} // namespace velora
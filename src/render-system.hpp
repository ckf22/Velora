#pragma once

#include "object-manager.hpp"
#include "movement-controller.hpp"
#include "descriptors.hpp"
#include "camera.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "point-light.hpp"

#define GLM_FORCE_RADIANS
#include <glm/common.hpp>

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
    struct UBO{
      glm::vec3 light_direction;
      float ambient;
      glm::vec3 light_color;
      u_int32_t point_light_count;
    };

    RenderSystem(Device& _device, MovementController& _movement_controller, Descriptors& _descriptor_manager, 
      const u_int32_t _frame_count, int width, int height);
    ~RenderSystem();

    RenderSystem& operator=(RenderSystem&) = delete;
    RenderSystem(RenderSystem&) = delete;

    u_int32_t get_vertex_count() { return objects.get_vertex_count(); }

    void allocate_from_descriptor_set();
    void apply_resize_to_camera(int width, int height);

    void update_shader_data(std::chrono::microseconds dt);
    void upload_shader_data(u_int32_t frame_index, bool force_upload = false);
    void populate_unique_buffers(VkCommandBuffer& cmd_buffer, u_int32_t frame_index, bool force_upload = false); // copy command has to be executed through a command buffer
    void populate_command_buffer(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout, u_int32_t frame_index);
  private:
    void push_constant_ranges(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout);
    void populate();
    void create_buffer_objects();

    u_int32_t frame_count;
    Device& device;
    MovementController& movement_controller;
    Descriptors& descriptor_manager;

    std::vector<std::unique_ptr<MyBuffer>> vertex_buffers;
    std::vector<std::unique_ptr<MyBuffer>> index_buffers;

    std::vector<std::unique_ptr<MyBuffer>> ssbo;
    std::vector<std::unique_ptr<MyBuffer>> ssbo_staging;

    std::vector<std::unique_ptr<MyBuffer>> ubo;
    std::vector<std::unique_ptr<MyBuffer>> ubo_staging;

    std::vector<std::unique_ptr<MyBuffer>> point_lights;
    std::vector<std::unique_ptr<MyBuffer>> point_lights_staging;


    float aspect_ratio;
    Camera camera{};
    ObjectManager objects{};
    UBO ubo_data{
      .light_direction = glm::normalize(glm::vec3{0,-10,-15}),
      .ambient = .1,
      .light_color = {.9f,.9f,.9f},
    };
    std::vector<PointLight> point_light_data;
};

} // namespace velora
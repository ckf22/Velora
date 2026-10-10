#pragma once

#include "sub-render-system.hpp"

#include <memory>

namespace velora{

class PointLightSystem;

class VertexRenderSystem : private SubRenderSystem{

    struct UBOData{
      glm::vec3 light_direction{0,-1,0};
      float ambient = .02f;
      glm::vec3 light_color{.9f};
      u_int32_t point_light_count = 0;
    };

    struct PushConstantsData{
      glm::mat4 projection{1.f}; // Identity matrix
    };

  public:

    VertexRenderSystem(
      std::shared_ptr<Device> _device, u_int32_t frame_count, VkFormat& image_format,
      VkFormat& depth_format, VkExtent2D extent, std::shared_ptr<PointLightSystem> _point_light_system
    );
    ~VertexRenderSystem(){}

    VertexRenderSystem(const VertexRenderSystem&) = delete;
    VertexRenderSystem& operator=(const VertexRenderSystem&) = delete;

    void update_data(std::chrono::nanoseconds dt, u_int32_t buffer_index, bool force_upload = false) override;
    void upload_data(u_int32_t buffer_index) override;

    void populate() override;

    void populate_command_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index) override;

    void update_projection_matrix(const glm::mat4& projection) override;
    void update_device_local_buffers(VkCommandBuffer& cmd_buffer, u_int32_t buffer_index) override;

    void allocate_descriptors() override;
    void register_descriptors() override;

  private:
    std::shared_ptr<PointLightSystem> point_light_system;

    UBOData ubo_data{};
    PushConstantsData push_constants_data{};
};

}
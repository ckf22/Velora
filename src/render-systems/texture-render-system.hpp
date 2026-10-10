#pragma once

#include "sub-render-system.hpp"
#include "point-light-system.hpp"
#include "texture-manager.hpp"

namespace velora{

class TextureRenderSystem : private SubRenderSystem {
    struct UBOData{
      glm::vec3 light_direction;
      float ambient;
      glm::vec3 light_color;
      u_int32_t point_light_count;

    };
    struct PushConstantsData{
      glm::mat4 projection;
      u_int32_t texture_index;
    };
  public:
    TextureRenderSystem(
      std::shared_ptr<Device> _device, std::shared_ptr<PointLightSystem> _point_light_system,
      const VkCommandPool& cmd_pool,
      VkFormat& image_format, VkFormat& depth_format, VkExtent2D extent, 
       const u_int32_t _frame_count
    );
    ~TextureRenderSystem();

    TextureRenderSystem(const TextureRenderSystem&) = delete;
    TextureRenderSystem& operator=(const TextureRenderSystem&) = delete;

    void update_data(std::chrono::nanoseconds dt, u_int32_t buffer_index, bool force_upload = false) override;
    void upload_data(u_int32_t buffer_index) override;

    void populate() override;

    void populate_command_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index) override;

    void update_projection_matrix(const glm::mat4& projection) override;
    void update_device_local_buffers(VkCommandBuffer& cmd_buffer, u_int32_t buffer_index) override;

  private:
    void allocate_descriptors() override;
    void register_descriptors() override;
    void load_textures();

    std::shared_ptr<PointLightSystem> point_light_system;

    std::unique_ptr<TextureManager> texture_manager;

    UBOData ubo_data{};
    PushConstantsData push_constant_data{};
    
    u_int32_t b_c = 0;
};

}
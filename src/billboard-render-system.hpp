#pragma once

#include "device.hpp"
#include "sub-render-system.hpp"

namespace velora{

class BillboardRenderSystem : private SubRenderSystem {
    struct Billboard{
      glm::vec3 world_pos;
      glm::vec3 color;
      float radius;

      static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions(){
        return { VkVertexInputAttributeDescription{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Billboard, world_pos)},
                 VkVertexInputAttributeDescription{.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Billboard, color)},
                 VkVertexInputAttributeDescription{.location = 2, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = offsetof(Billboard, radius)} };
      }
      static std::vector<VkVertexInputBindingDescription> get_binding_descriptions(){
        return { VkVertexInputBindingDescription{.binding = 0, .stride = sizeof(Billboard), .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE} };
      }
    };
  public:
    struct PushConstantsData{
      glm::mat4 projection_view;
      glm::vec3 camera_up;
      float cut_off_distance_squared;
      glm::vec3 camera_right;
  };

    BillboardRenderSystem(
      std::shared_ptr<Device> _device, u_int32_t frame_count, VkFormat& image_format,
      VkFormat& depth_format, VkExtent2D extent
    );
    ~BillboardRenderSystem();

    BillboardRenderSystem(const BillboardRenderSystem&) = delete;
    BillboardRenderSystem& operator=(const BillboardRenderSystem&) = delete;

    void update_data(std::chrono::nanoseconds dt, u_int32_t buffer_index, bool force_upload = false) override;
    void upload_data(u_int32_t buffer_index) override;

    void populate() override;

    void populate_command_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index) override;

    void update_projection_matrix(const glm::mat4& projection) override;
    void update_device_local_buffers(VkCommandBuffer& cmd_buffer, u_int32_t buffer_index) override;

    void update_camera_vectors(const glm::mat4& view);

    void allocate_descriptors() override;
    void register_descriptors() override;
  private:
    std::vector<Billboard> data;

    PushConstantsData push_constants{};
};

}
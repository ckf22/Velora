#pragma once

#include "point-light.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "descriptors.hpp"

#include <vector>
#include <memory>

namespace velora{


class PointLightSystem{
  public:
    PointLightSystem(std::shared_ptr<Device> _device, const u_int32_t _max_point_light_count, const u_int32_t max_frames_in_flight);
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem&) = delete;
    PointLightSystem& operator=(const PointLightSystem&) = delete;

    const VkDescriptorSetLayout& get_descriptor_layout() { return this->descriptors->get_layout(); }

    std::vector<PointLight>& get_point_lights() { return this->point_lights; }
    u_int32_t get_point_light_count() { return static_cast<u_int32_t>(this->point_lights.size()); }

    void upload_to_buffer(VkCommandBuffer& cmd_buffer, u_int32_t target_index);

    void bind_set(VkCommandBuffer& cmd_buffer, VkPipelineLayout& layout, u_int32_t index);
  private:
    std::shared_ptr<Device> device;
    const u_int32_t max_point_light_count;

    // In Bytes
    const u_int32_t buffer_segment_size, buffer_segment_count;

    std::vector<PointLight> point_lights;

    // These buffers will be split into MAX_FRAMES_IN_FLIGHT segments
    std::unique_ptr<MyBuffer> staging;
    std::unique_ptr<MyBuffer> data;

    std::unique_ptr<Descriptors> descriptors;
};

}
#pragma once

#include "buffer.hpp"
#include "descriptors.hpp"
#include "object-manager.hpp"
#include "pipeline.hpp"
#include "device.hpp"

#define GLM_FORCE_RADIANS
#include <glm/mat4x4.hpp>

#include <chrono>
#include <memory>
#include <vector>


namespace velora{

// This class is an interface
// The derived classes handle GPU buffer objects, hold the objects in VRAM and
// are reponsible for rendering their contents by populating a given VkCommandBuffer
class SubRenderSystem{
  public:
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif

    struct PushConstantsData;
    struct UBOData;

    SubRenderSystem(std::shared_ptr<Device> _device, const u_int32_t _frame_count) : device{_device}, frame_count{_frame_count} {}
    ~SubRenderSystem();

    SubRenderSystem(const SubRenderSystem&) = delete;
    SubRenderSystem& operator=(const SubRenderSystem&) = delete;

    virtual void update_projection_matrix(const glm::mat4& projection) = 0;

    // Only run after Descriptor Sets have been created
    virtual void allocate_descriptors() = 0;

    // Loads and adds objects, temp: until ECS
    virtual void populate() = 0;

    // Only update data that has changed
    virtual void update_data(std::chrono::nanoseconds dt, u_int32_t buffer_index, bool force_upload = false) = 0;

    // Upload all data
    virtual void upload_data(u_int32_t buffer_index) = 0;

    // Device local buffers need to be updated through a VkCommandBuffer
    virtual void update_device_local_buffers(VkCommandBuffer& cmd_buffer, u_int32_t buffer_index) = 0;

    // Binds descriptors and adds Draw Calls
    virtual void populate_command_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index) = 0;

    virtual void register_descriptors() = 0;

    virtual void create_buffer_objects(u_int32_t buffer_count, u_int32_t vertex_bytes, u_int32_t index_bytes, u_int32_t ssbo_bytes, u_int32_t ubo_bytes);

    std::shared_ptr<Device> device;
    const u_int32_t frame_count;

    std::unique_ptr<Descriptors> descriptor_manager;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<ObjectManager> objects; // temp until ECS

    std::vector<std::unique_ptr<MyBuffer>> vertex_buffers;
    std::vector<std::unique_ptr<MyBuffer>> index_buffers;

    std::vector<std::unique_ptr<MyBuffer>> ssbo;
    std::vector<std::unique_ptr<MyBuffer>> ssbo_staging;

    std::vector<std::unique_ptr<MyBuffer>> ubo;
    std::vector<std::unique_ptr<MyBuffer>> ubo_staging;
};

}
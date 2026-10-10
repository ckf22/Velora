#include "billboard-render-system.hpp"

namespace velora{

BillboardRenderSystem::BillboardRenderSystem(
    std::shared_ptr<Device> _device, u_int32_t frame_count, VkFormat& image_format,
    VkFormat& depth_format, VkExtent2D extent
) : SubRenderSystem{_device, frame_count} {

    this->populate();

    this->pipeline = std::make_unique<Pipeline>(
        _device, std::vector<VkDescriptorSetLayout>{},
        std::string{"./shaders/billboard-shader.vert.spv"}, std::string{"./shaders/billboard-shader.frag.spv"},
        Billboard::get_attribute_descriptions(), Billboard::get_binding_descriptions(),
        extent, sizeof(PushConstantsData), image_format, depth_format
    );
}

BillboardRenderSystem::~BillboardRenderSystem(){}

void BillboardRenderSystem::update_data(std::chrono::nanoseconds dt, u_int32_t buffer_index, bool force_upload){
    if(force_upload){
        auto dest = this->vertex_buffers.at(buffer_index)->map();

        u_int32_t copy_size = std::min(this->vertex_buffers.at(buffer_index)->get_size(),
                                         static_cast<u_int32_t>(this->data.size()*sizeof(Billboard)));

        memcpy(dest, this->data.data(), copy_size);

        this->vertex_buffers.at(buffer_index)->unmap();
    }
}

void BillboardRenderSystem::upload_data(u_int32_t buffer_index){
    this->update_data(std::chrono::nanoseconds{0}, buffer_index, true);
}

void BillboardRenderSystem::populate(){
    this->data = {
        {{0,-10,0}, {81.0/256, 27.0/256, 133.0/256}, .75},
    };

    this->push_constants.cut_off_distance_squared = 1;

    for(int i = 0; i < this->frame_count; ++i){
        this->vertex_buffers.push_back( 
            std::make_unique<MyBuffer>(
                this->device, sizeof(Billboard), this->data.size(),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
            )
        );
    }
}

void BillboardRenderSystem::populate_command_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index){
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->get_pipeline());

    VkDeviceSize offset{0};
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &this->vertex_buffers.at(index)->get_buffer(), &offset);

    vkCmdPushConstants(
        cmd_buffer, this->pipeline->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(PushConstantsData), &this->push_constants
    );

    // One center point will be provided per billboard, the shader will run 6 times two form two triangles
    vkCmdDraw(cmd_buffer, 6, this->data.size(), 0, 0);
}

void BillboardRenderSystem::update_projection_matrix(const glm::mat4& projection){
    this->push_constants.projection_view = projection;
}

void BillboardRenderSystem::update_camera_vectors(const glm::mat4& view){
    this->push_constants.camera_right = glm::vec3{view[0][0], view[1][0], view[2][0]};
    this->push_constants.camera_up = glm::vec3{view[0][1], view[1][1], view[2][1]};
}

void BillboardRenderSystem::update_device_local_buffers(VkCommandBuffer& cmd_buffer, u_int32_t buffer_index){
    throw std::runtime_error("Functionality not implemented");
}

void BillboardRenderSystem::allocate_descriptors(){
    throw std::runtime_error("Functionality not implemented");
}

void BillboardRenderSystem::register_descriptors(){
    throw std::runtime_error("Functionality not implemented");
}



}
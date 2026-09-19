#include "texture-render-system.hpp"

namespace velora{

TextureRenderSystem::TextureRenderSystem(
  std::shared_ptr<Device> _device, std::shared_ptr<PointLightSystem> _point_light_system,
  const VkCommandPool& cmd_pool,
  VkFormat& image_format, VkFormat& depth_format, VkExtent2D extent, 
  const u_int32_t _frame_count
) : SubRenderSystem(_device, _frame_count), point_light_system{_point_light_system} {

    this->populate();

    this->create_buffer_objects(
        _frame_count, this->objects->get_max_vertex_count() * sizeof(Vertex),
        this->objects->get_max_index_count() * sizeof(u_int32_t),
        1000 * sizeof(glm::mat4), sizeof(UBOData)
    );

    this->register_descriptors();

    this->textures = std::make_unique<TextureManager>(_device, *this->descriptor_manager, cmd_pool, std::string{"./assets/brick-texture.png"});

    this->pipeline = std::make_unique<Pipeline>(
        _device, std::vector<VkDescriptorSetLayout>{this->descriptor_manager->generate_layout(), _point_light_system->get_descriptor_layout()},
        std::string{"./shaders/ssbo-3d-shader.vert.spv"}, std::string{"./shaders/texture-shader.frag.spv"},
        Vertex::get_attribute_descriptions(), Vertex::get_binding_descriptions(),
        extent, sizeof(PushConstantsData), image_format, depth_format
    );

    this->allocate_descriptors();
    this->textures->allocate_descriptors(*this->descriptor_manager);


    this->ubo_data = {
        .light_direction = {0,-1,0},
        .ambient = .1f,
        .light_color = {.9f,.9f,.9f},
        .point_light_count = 0
    };
}

TextureRenderSystem::~TextureRenderSystem(){}

void TextureRenderSystem::update_data(std::chrono::nanoseconds dt, u_int32_t buffer_index, bool force_upload){
    this->objects->upload_shader_data(
        this->vertex_buffers.at(buffer_index)->map(),
        this->index_buffers.at(buffer_index)->map(),
        this->vertex_buffers.at(buffer_index)->get_size(),
        this->index_buffers.at(buffer_index)->get_size(),
        force_upload
    );

    this->vertex_buffers.at(buffer_index)->unmap();
    this->index_buffers.at(buffer_index)->unmap();
}

void TextureRenderSystem::upload_data(u_int32_t buffer_index){
    this->update_data(std::chrono::nanoseconds{0}, buffer_index, true);
}

void TextureRenderSystem::populate(){
    this->objects = std::make_unique<ObjectManager>();

    std::vector<Vertex> vertices{
        {{-5,2,-5}, {1,1,1}, {0,-1,0}, {0,0}},
        {{5,2,-5},  {1,1,1}, {0,-1,0}, {1,0}},
        {{-5,2,5},  {1,1,1}, {0,-1,0}, {0,1}},
        {{5,2,5},   {1,1,1}, {0,-1,0}, {1,1}},
    };
    std::vector<u_int32_t> indices{0,1,2,2,1,3};
    Object o{vertices, indices, static_cast<u_int32_t>(vertices.size()), static_cast<u_int32_t>(indices.size())};
    this->objects->add_object(std::move(o));
}

void TextureRenderSystem::populate_command_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index){
    // Bind Pipeline
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->get_pipeline());

    // Update Push Constants
    vkCmdPushConstants(
        cmd_buffer, this->pipeline->get_pipeline_layout(),
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, sizeof(PushConstantsData), &this->push_constant_data
    );

    // Bind descriptors
    this->descriptor_manager->bind_descriptor_set(cmd_buffer, this->pipeline->get_pipeline_layout(), index);
    this->point_light_system->bind_set(cmd_buffer, this->pipeline->get_pipeline_layout(), index);

    VkDeviceSize offset{0};
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &this->vertex_buffers.at(index)->get_buffer(), &offset);
    vkCmdBindIndexBuffer(cmd_buffer, this->index_buffers.at(index)->get_buffer(), offset, VK_INDEX_TYPE_UINT32);

    // Add Draw Calls
    this->objects->add_draw_calls(cmd_buffer);
}

void TextureRenderSystem::update_projection_matrix(const glm::mat4& projection){
    this->push_constant_data.projection = projection;
}

void TextureRenderSystem::update_device_local_buffers(VkCommandBuffer& cmd_buffer, u_int32_t buffer_index){
    this->ubo_data.point_light_count = this->point_light_system->get_point_light_count();
    // UBO
    memcpy(this->ubo_staging.at(buffer_index)->map(), &this->ubo_data, sizeof(UBOData));
    this->ubo_staging.at(buffer_index)->unmap();

    VkBufferCopy ubo_region{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = sizeof(UBOData),
    };
    vkCmdCopyBuffer(cmd_buffer, this->ubo_staging.at(buffer_index)->get_buffer(), this->ubo.at(buffer_index)->get_buffer(), 1, &ubo_region);


    // SSBO
    u_int32_t buffer = this->objects->upload_transforms(this->ssbo_staging.at(buffer_index)->map(), -1, true);
    this->ssbo_staging.at(buffer_index)->unmap();

    VkBufferCopy ssbo_region{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = buffer,
    };
    vkCmdCopyBuffer(cmd_buffer, this->ssbo_staging.at(buffer_index)->get_buffer(), this->ssbo.at(buffer_index)->get_buffer(), 1, &ssbo_region);
}

void TextureRenderSystem::allocate_descriptors(){
    this->descriptor_manager->generate_sets();

    VkDescriptorBufferInfo ubo_info{
        .buffer = nullptr,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    VkWriteDescriptorSet write_ubo{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .pBufferInfo = &ubo_info,
    };

    VkDescriptorBufferInfo ssbo_info{
        .buffer = nullptr,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    VkWriteDescriptorSet write_ssbo{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 2,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .pBufferInfo = &ssbo_info,
    };


    for(int i = 0; i < this->frame_count; ++i){
        ubo_info.buffer = this->ubo.at(i)->get_buffer();
        ssbo_info.buffer = this->ssbo.at(i)->get_buffer();

        this->descriptor_manager->allocate_descriptor(write_ubo, i);
        this->descriptor_manager->allocate_descriptor(write_ssbo, i);
    }
}

void TextureRenderSystem::register_descriptors(){
    // 0 is the set=0 specifier in the shaders
    this->descriptor_manager = std::make_unique<Descriptors>(this->device, this->frame_count, 0);

   // UBO
    this->descriptor_manager->add_binding(
        VkDescriptorSetLayoutBinding{
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        }
    );

    // World Space Transforms
    this->descriptor_manager->add_binding(
        VkDescriptorSetLayoutBinding{
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        }
    );

}



}
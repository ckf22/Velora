#include "render-system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <iostream>
#include <string.h>

namespace velora{

RenderSystem::RenderSystem(Device& _device, MovementController& _movement_controller, u_int32_t _frame_count, int width, int height)
 : device{_device}, frame_count{_frame_count}, movement_controller{_movement_controller} {
    this->populate();
    this->create_vertex_buffers();


    int i = 0;
    for(int i = 0; i < _frame_count; ++i){
        this->ssbo.push_back( std::make_unique<MyBuffer>(_device, 1000, 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ) );
        this->ssbo_staging.push_back( std::make_unique<MyBuffer>(_device, this->ssbo[i]->get_size(), 1,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        ) );
    }

    this->camera.view_angles({0, -.0f, -10}, {.0f, 0.f, .0f});
    this->register_resize(width, height);
}

RenderSystem::~RenderSystem(){
}

void RenderSystem::fill_ssbo(VkCommandBuffer& cmd_buffer, u_int32_t frame_index){
    TransformComponent t({0,0,0},{1,2,1});
    auto dest = this->ssbo_staging[frame_index]->map();
    glm::mat4 data = t.get_transform();
    memcpy(dest, &data, sizeof(glm::mat4));
    this->ssbo_staging[frame_index]->unmap();

    VkBufferCopy copy{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = 1000
    };
    vkCmdCopyBuffer(cmd_buffer, this->ssbo_staging[frame_index]->get_buffer(), this->ssbo[frame_index]->get_buffer(), 1, &copy);
}

void RenderSystem::fill_command_buffer(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout, VkDescriptorSet& set, u_int32_t frame_index){
    this->upload_shader_data(frame_index);

    this->bind_buffer_objects(cmd_buffer, frame_index);

    u_int32_t o = 0;
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,
        0, 1, &set, 1, &o);

    this->push_constant_ranges(cmd_buffer, pipeline_layout);

    // the actual magic
    vkCmdDraw(cmd_buffer, this->get_vertex_count(), 1, 0, 0);

}

void RenderSystem::add_ssbo_descriptor_set(DescriptorPool& pool){
    for(auto& it : this->ssbo)
        pool.add_b(*it);
}

void RenderSystem::register_resize(int width, int height){
    this->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    this->camera.perspective_projection(.1f, 50.f, 60, this->aspect_ratio);
}

void RenderSystem::update_shader_data(std::chrono::microseconds dt){ // dt is short for delta time
    this->movement_controller.apply_to_camera(camera);
}

void RenderSystem::upload_shader_data(u_int32_t frame_index){
    auto dest = this->vertex_buffers[frame_index]->map();
    this->vertex_buffers[frame_index]->map();
    auto a = this->objects.upload_shader_data(dest, this->vertex_buffers[frame_index]->get_size());

    //if constexpr (debug)
    //    std::cout << "Copied " << a << " Bytes" << std::endl;
}

void RenderSystem::push_constant_ranges(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout){
    PushConstantRange push{
        .perspective_projection = this->camera.get_projection_view_matrix(),
        .worldspace_transformation = this->transform.get_transform(true)
    };
    vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
         0, sizeof(PushConstantRange), &push);
}

void RenderSystem::bind_buffer_objects(VkCommandBuffer& cmd_buffer, u_int32_t frame_index){
    VkDeviceSize offsets{0};
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &this->vertex_buffers[frame_index]->get_buffer(), &offsets);
}

void RenderSystem::populate(){
    auto buffer = Vertex::get_default_cube();
    Object _object{buffer, static_cast<u_int32_t>(buffer.size())};
    this->objects.add_object(_object);

    auto buffer2 = Vertex::get_default_cube({1,1,1});
    Object __object{buffer2, static_cast<u_int32_t>(buffer2.size())};
    this->objects.add_object(__object);

    auto buffer3 = Vertex::get_default_cube({1,2.1f,1});
    Object ___object{buffer3, static_cast<u_int32_t>(buffer3.size())};
    this->objects.add_object(___object);

    // plane at the bottom
    std::vector<Vertex> buffer4 = {
        {{-10, 2, -10},{.1f,.9f,.9f}},
        {{-10, 2, 10},{.9f,.9f,.1f}},
        {{10, 2, -10},{.9f,.1f,.9f}},

        {{10, 2, 10},{.1f,.9f,.9f}},
        {{-10, 2, 10},{.9f,.9f,.1f}},
        {{10, 2, -10},{.9f,.1f,.9f}},
    };
    Object ____object(buffer4, static_cast<u_int32_t>(buffer4.size()));
    this->objects.add_object(____object);
}


void RenderSystem::create_vertex_buffers(){

    this->vertex_buffers.clear();
    u_int32_t buffer_size = sizeof(Vertex);
    u_int32_t count = 2000; // abitrarily high number so the data will fit

    for(int i = 0; i < this->frame_count; ++i){
        this->vertex_buffers.push_back( std::make_unique<MyBuffer>(this->device, buffer_size, count,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) );
    }

    if constexpr (debug)
        std::cout << "Vertex Buffers created" << std::endl;

}


} // namespace velora
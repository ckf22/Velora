#include "render-system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <iostream>
#include <string.h>

namespace velora{

RenderSystem::RenderSystem(Device& _device, MovementController& _movement_controller, Descriptors& _descriptor_manager, const u_int32_t _frame_count, int width, int height)
 : device{_device}, frame_count{_frame_count}, movement_controller{_movement_controller}, descriptor_manager{_descriptor_manager} {
    this->populate();
    this->create_buffer_objects();

    this->descriptor_manager.add_binding( 
      VkDescriptorSetLayoutBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_ALL
      }
    );
    this->descriptor_manager.add_binding( 
      VkDescriptorSetLayoutBinding{
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
      }
    );
    this->descriptor_manager.add_binding( 
      VkDescriptorSetLayoutBinding{
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
      }
    );


    this->camera.view_angles({0, -.5f, -10}, {.0f, 0.f, .0f});
    this->apply_resize_to_camera(width, height);
}

RenderSystem::~RenderSystem(){
}

void RenderSystem::populate_unique_buffers(VkCommandBuffer& cmd_buffer, u_int32_t frame_index, bool force_upload){
    if( !force_upload )
        return;

    auto dest = this->ssbo_staging[frame_index]->map();
    this->objects.upload_transforms(dest, -1, true);
    this->ssbo_staging[frame_index]->unmap();

    VkBufferCopy copy{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = this->ssbo_staging[frame_index]->get_size()
    };
    vkCmdCopyBuffer(cmd_buffer, this->ssbo_staging[frame_index]->get_buffer(), this->ssbo[frame_index]->get_buffer(), 1, &copy);


    dest = this->ubo_staging[frame_index]->map();
    memcpy(dest, (void*)&this->ubo_data, sizeof(UBO));
    this->ubo_staging[frame_index]->unmap();

    VkBufferCopy copy2{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = this->ubo_staging[frame_index]->get_size()
    };
    vkCmdCopyBuffer(cmd_buffer, this->ubo_staging[frame_index]->get_buffer(), this->ubo[frame_index]->get_buffer(), 1, &copy2);

    dest = this->point_lights_staging[frame_index]->map();


    memcpy(dest, this->point_light_data.data(), this->point_light_data.size()*sizeof(PointLight));
    this->point_lights_staging[frame_index]->unmap();

    VkBufferCopy copy3{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = this->point_lights_staging[frame_index]->get_size()
    };
    vkCmdCopyBuffer(cmd_buffer, this->point_lights_staging[frame_index]->get_buffer(),
        this->point_lights[frame_index]->get_buffer(), 1, &copy3);
}

void RenderSystem::populate_command_buffer(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout, u_int32_t frame_index){
    VkDeviceSize offset{0};
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &this->vertex_buffers[frame_index]->get_buffer(), &offset);
    vkCmdBindIndexBuffer(cmd_buffer, this->index_buffers[frame_index]->get_buffer(), offset, VK_INDEX_TYPE_UINT32);

    this->push_constant_ranges(cmd_buffer, pipeline_layout);

    this->objects.add_draw_calls(cmd_buffer);
}

void RenderSystem::allocate_from_descriptor_set(){
    VkDescriptorBufferInfo info_ubo{
        .buffer = this->ubo[0]->get_buffer(),
        .offset = 0,
        .range = this->ubo[0]->get_size()
    };
    VkWriteDescriptorSet write_ubo{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .pBufferInfo = &info_ubo,
    };


    VkDescriptorBufferInfo info_ssbo{
        .buffer = this->ssbo[0]->get_buffer(),
        .offset = 0,
        .range = this->ssbo[0]->get_size()
    };
    VkWriteDescriptorSet write_ssbo{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 2,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .pBufferInfo = &info_ssbo,
    };


    VkDescriptorBufferInfo info_point_light{
        .buffer = this->point_lights[0]->get_buffer(),
        .offset = 0,
        .range = this->point_lights[0]->get_size()
    };
    VkWriteDescriptorSet write_point_light{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 3,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .pBufferInfo = &info_point_light,
    };

    for(int i = 0; i < this->frame_count; ++i){
        info_ubo.buffer = this->ubo[i]->get_buffer();
        info_ubo.range = this->ubo[i]->get_size();

        info_ssbo.buffer = this->ssbo[i]->get_buffer();
        info_ssbo.range = this->ssbo[i]->get_size();

        info_point_light.buffer = this->point_lights[i]->get_buffer();
        info_point_light.range = this->point_lights[i]->get_size();

        this->descriptor_manager.allocate_descriptor(write_ubo, i);
        this->descriptor_manager.allocate_descriptor(write_ssbo, i);
        this->descriptor_manager.allocate_descriptor(write_point_light, i);
    }
}

void RenderSystem::apply_resize_to_camera(int width, int height){
    this->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    this->camera.perspective_projection(.1f, 1000.f, 60, this->aspect_ratio);
}

void RenderSystem::update_shader_data(std::chrono::microseconds dt){ // dt is short for delta time
    this->movement_controller.apply_to_camera(camera);

    this->objects.update_shader_data(dt);
}

void RenderSystem::upload_shader_data(u_int32_t frame_index, bool force_upload){
    auto a = this->objects.upload_shader_data(
        this->vertex_buffers[frame_index]->map(),
        this->index_buffers[frame_index]->map(),
        this->vertex_buffers[frame_index]->get_size(),
        this->index_buffers[frame_index]->get_size(),
        force_upload
    );

    //if constexpr (debug)
    //    std::cout << "Copied " << a << " Bytes" << std::endl;
}

void RenderSystem::push_constant_ranges(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout){
    PushConstantRange push{
        .perspective_projection = this->camera.get_projection_view_matrix(),
    };
    vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
         0, sizeof(PushConstantRange), &push);
}

void RenderSystem::populate(){
    //Object perlin_noise = Object::perlin_noise({4.f,10.7f,4.f},3,40,20);
    //perlin_noise.get_transforms() = { TransformComponent{{-20,0,-20},{10,10,10}} };
    //this->objects.add_object(std::move(perlin_noise));
    /*Object buffer2 = Object::load_file("models/smooth_vase.obj");
    buffer2.get_transforms() = { 
        TransformComponent{{3,0,0},{10,10,10}}, TransformComponent{{0,0,0},{10,-10,10},{glm::radians(glm::vec3{0,0,180})}},
        TransformComponent{{-3,0,0}, {10,10,10}}
    };
    this->objects.add_object(std::move(buffer2));*/

    //Object buffer3 = Object::load_file("models/flat_vase.obj");
    //buffer3.get_transforms() = { TransformComponent{{-3,0,0},{10,10,10}} };
    //this->objects.add_object(std::move(buffer3));

    //this->objects.load_file("models/high-res-apple.obj", {TransformComponent({6.f,-2.f,0.f},{20,-20,20},{0.1,1.2,0.6})} );
    //this->objects.load_file("models/mcx-spear-eft.obj", {TransformComponent({-2,-8,0},{20,20,20},{glm::radians(glm::vec3{-20,160, -20})})} );
    //this->objects.load_file("./models/terrain-mesh.obj", { TransformComponent{{-10,0,-10},{2,2,2},{glm::radians(90.f),0,0}} }  );

    // plane at the bottom
    std::vector<Vertex> buffer5 = {
        {{-5, 0, -5},{.1f,.9f,.9f}, {0,-1,0}, {0,0}},
        {{-5, 0, 5},{.9f,.9f,.1f},  {0,-1,0}, {1,0}},
        {{5, 0, -5},{.9f,.1f,.9f},  {0,-1,0}, {0,1}},
        {{5, 0, 5},{.1f,.9f,.9f},   {0,-1,0}, {1,1}}
    };
    std::vector<u_int32_t> indices = {0,1,2,1,2,3};
    Object object4(buffer5, indices, static_cast<u_int32_t>(buffer5.size()), static_cast<u_int32_t>(indices.size()));
    object4.get_transforms() = { TransformComponent{}, TransformComponent{{0,0,0}, {1,1,1}, {glm::radians(-30.f),0,0}, {0,0,5}} };
    this->objects.add_object(std::move(object4));

    //this->point_light_data = {
    //    PointLight{.position = {0,-5,0}, .intensity = 12.f, .color = {.1f,.2f,.9f}, .range = 20},
    //    PointLight{.position = {0,0,5}, .intensity = 15.f, .color = {1,.2f,.2f}, .range = 15}
    //};
    //this->ubo_data.light_color = {.3,.3,.5};
    this->point_light_data = { PointLight{.position = {-2,-20,-2}, .intensity = 150, .color = {.9,.85,.75}} };
    this->ubo_data.point_light_count = static_cast<u_int32_t>(this->point_light_data.size());
}


void RenderSystem::create_buffer_objects(){

    this->vertex_buffers.clear();
    u_int32_t buffer_size = sizeof(Vertex);
    u_int32_t count = this->objects.get_max_vertex_count();

    for(int i = 0; i < this->frame_count; ++i){
        this->vertex_buffers.push_back( 
            std::make_unique<MyBuffer>(
                this->device, buffer_size, count,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 
            )
        );
        this->index_buffers.push_back(
            std::make_unique<MyBuffer>(
                this->device, sizeof(u_int32_t), this->objects.get_max_index_count(),
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            )
        );
        this->ssbo.push_back(
            std::make_unique<MyBuffer>(
                this->device, sizeof(glm::mat4), 1000, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            )
        );
        this->ssbo_staging.push_back(
            std::make_unique<MyBuffer>(
                this->device, this->ssbo[i]->get_size(), 1,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            ) 
        );
        this->ubo.push_back(
            std::make_unique<MyBuffer>(
                this->device, sizeof(UBO), 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            )
        );
        this->ubo_staging.push_back(
            std::make_unique<MyBuffer>(
                this->device, this->ubo[i]->get_size(), 1,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            ) 
        );
        this->point_lights.push_back(
            std::make_unique<MyBuffer>(
                this->device, sizeof(PointLight), this->point_light_data.size()+1,  // as buffer for the count variable
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            )
        );
        this->point_lights_staging.push_back(
            std::make_unique<MyBuffer>(
                this->device, this->point_lights[i]->get_size(), 1,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            )
        );
    }

    if constexpr (debug)
        std::cout << "Vertex Buffers created\n" 
            << static_cast<float>( (
                    this->vertex_buffers[0]->get_size()
                    +this->index_buffers[0]->get_size()
                    +(this->ssbo_staging[0]->get_size()*2)
                    +(this->point_lights[0]->get_size()*2)
                ) * this->frame_count 
            ) / 1000000 
            << "MB of RAM Used" << std::endl;

}


} // namespace velora
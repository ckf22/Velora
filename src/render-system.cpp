#include "render-system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <iostream>

namespace velora{

RenderSystem::RenderSystem(Device& _device, u_int32_t _frame_count, int width, int height) : device{_device}, frame_count{_frame_count} {
    this->populate();
    this->create_vertex_buffers();

    //    this->test = std::make_unique<MyBuffer>(_device, 1024, 65536, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //    VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);

    //this->camera.orthographic_projection(-5, 15, 3, -3, -3, 3);
    this->camera.view_direction({-2, -.5f, -10}, {.5f, 0.f, .9f}, {.0f, -1.f, .0f});
    this->register_resize(width, height);
}

RenderSystem::~RenderSystem(){
}

void RenderSystem::register_resize(int width, int height){
    this->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    this->camera.perspective_projection(.1f, 50.f, 60, this->aspect_ratio);
}

void RenderSystem::update_shader_data(std::chrono::microseconds dt){ // dt is short for delta time
    this->transform.rotation.y = glm::mod( this->transform.rotation.y + (.002f*(dt.count()/1024))  , glm::two_pi<float>());
    this->transform.rotation.z = glm::mod( this->transform.rotation.z + (.0008f*(dt.count()/1024)) , glm::two_pi<float>());
    this->transform.rotation.x = glm::mod( this->transform.rotation.x + (.0002f*(dt.count()/1024)) , glm::two_pi<float>());

    this->dy += (  .003f * (dt.count()/1024)  );
    this->transform.translation.x = glm::sin(this->dy)*3;
}

void RenderSystem::upload_shader_data(u_int32_t frame_index){
    auto dest = this->vertex_buffers[frame_index]->map();
    this->vertex_buffers[frame_index]->map();
    auto a = this->objects.upload_shader_data(dest, this->vertex_buffers[frame_index]->get_size());
    //this->vertex_buffers[frame_index]->unmap();

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

void RenderSystem::bind(VkCommandBuffer& cmd_buffer, u_int32_t frame_index){
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
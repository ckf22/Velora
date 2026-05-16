#include "render-system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <iostream>

namespace velora{

RenderSystem::RenderSystem(Device& _device, u_int32_t _frame_count, int width, int height) : device{_device}, frame_count{_frame_count} {
    this->create_buffers();
    this->populate();

    //this->camera.orthographic_projection(-5, 15, 3, -3, -3, 3);
    this->register_resize(width, height);
}

RenderSystem::~RenderSystem(){
    for(auto& it : this->vertex_buffers)
        vkDestroyBuffer(this->device.get_device(), it, nullptr);

    for(auto& it : this->vertex_buffer_memory)
        vkFreeMemory(this->device.get_device(), it, nullptr);
}

void RenderSystem::register_resize(int width, int height){
    this->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    this->camera.perspective_projection(.1f, 20.f, 60, this->aspect_ratio);
}

void RenderSystem::update_shader_data(std::chrono::microseconds dt){ // dt is short for delta time
    this->transform.rotation.y = glm::mod( this->transform.rotation.y + (.002f*(dt.count()/1024))  , glm::two_pi<float>());
    this->transform.rotation.z = glm::mod( this->transform.rotation.z + (.0008f*(dt.count()/1024)) , glm::two_pi<float>());
    this->transform.rotation.x = glm::mod( this->transform.rotation.x + (.0002f*(dt.count()/1024)) , glm::two_pi<float>());

    this->dy += (  .003f * (dt.count()/1024)  );
    this->transform.translation.x = glm::sin(this->dy)*3;
}

void RenderSystem::upload_shader_data(u_int32_t frame_index){
    void * dest;
    if( vkMapMemory(this->device.get_device(), this->vertex_buffer_memory[frame_index], 0, this->buffer_size, 0, &dest) != VK_SUCCESS)
        throw std::runtime_error("Failed to map memory");
    auto a = this->objects.upload_shader_data(dest, this->buffer_size);
    vkUnmapMemory(this->device.get_device(), this->vertex_buffer_memory[frame_index]);

    if constexpr (debug)
        std::cout << "Copied " << a << " Bytes" << std::endl;
}

void RenderSystem::push_constant_ranges(VkCommandBuffer& cmd_buffer, VkPipelineLayout& pipeline_layout){
    PushConstantRange push{
        .perspective_projection = this->camera.projection,
        .worldspace_transformation = this->transform.get_transform(true)
    };
    vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
         0, sizeof(PushConstantRange), &push);
}

void RenderSystem::bind(VkCommandBuffer& cmd_buffer, u_int32_t frame_index){
    VkDeviceSize offsets{0};
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &this->vertex_buffers[frame_index], &offsets);
}

void RenderSystem::populate(){
    auto buffer = Vertex::get_default_cube();
    Object _object{buffer, static_cast<u_int32_t>(buffer.size())};
    this->objects.add_object(_object);

    /*auto buffer2 = Vertex::get_default_cube({1,1,1});
    Object __object{buffer2, static_cast<u_int32_t>(buffer2.size())};
    this->objects.add_object(__object);*/
}


void RenderSystem::create_buffers(){

    this->vertex_buffers.resize(this->frame_count);
    this->vertex_buffer_memory.resize(this->frame_count);
    this->buffer_size = sizeof(Vertex)*2000; // abitrarily high number so the data will fit

    for(int i = 0; i < this->frame_count; ++i){
        VkBufferCreateInfo buffer_ci{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = this->buffer_size,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        if( vkCreateBuffer(this->device.get_device(), &buffer_ci, VK_NULL_HANDLE, &this->vertex_buffers[i]) != VK_SUCCESS )
            throw std::runtime_error("Failed to create vertex buffer");


        VkMemoryRequirements ram_requirements;
        vkGetBufferMemoryRequirements(this->device.get_device(), this->vertex_buffers[i], &ram_requirements);

        VkMemoryAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = ram_requirements.size,
            .memoryTypeIndex = 
            this->device.find_memory_type(ram_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        };

        if( vkAllocateMemory(this->device.get_device(), &alloc_info, nullptr, &this->vertex_buffer_memory[i]) != VK_SUCCESS )
            throw std::runtime_error("Failed to allocate Vertex Buffer Memory");

        vkBindBufferMemory(this->device.get_device(), this->vertex_buffers[i], this->vertex_buffer_memory[i], 0);
    }
    if constexpr (debug)
        std::cout << "Vertex Buffers created" << std::endl;

}


} // namespace velora
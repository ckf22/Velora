#include "object-manager.hpp"

#include <stdexcept>
#include <iostream>
#include <string.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace velora{

ObjectManager::ObjectManager(Device& _device, const u_int32_t _frame_count, std::vector<Vertex> object) : device{_device}, frame_count{_frame_count} {
    this->vertices = object;
    this->vertex_count = object.size();

    if( this->vertex_count % 3 != 0 )
        std::cout << "vertex count out of 3-alignment" << std::endl;

    this->create_buffer();
}
ObjectManager::~ObjectManager(){
    for(auto& it : this->vertex_buffers)
        vkDestroyBuffer(this->device.get_device(), it, nullptr);
    for(auto& it : this->vertex_buffer_memory)
        vkFreeMemory(this->device.get_device(), it, nullptr);
}

void ObjectManager::create_buffer(){

    this->vertex_buffers.resize(this->frame_count);
    this->vertex_buffer_memory.resize(this->frame_count);
    for(int i = 0; i < this->frame_count; ++i){
        VkBufferCreateInfo buffer_ci{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(Vertex)*1000,  // abitrarily high number so the data will fit
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

void ObjectManager::push_constant_ranges(VkCommandBuffer& cmd_buffer, VkPipelineLayout& layout){
    this->rotation_z = glm::mod(this->rotation_z + .02f, glm::two_pi<float>());
    this->tranform.anchor = {0,0,0.5};
    this->tranform.rotation = {0.2, rotation_z, 0.1};
    this->tranform.scale = {1,1,1};

    ConstantRanges push{.projection=this->tranform.get_transform(true), .ambient_light={0.0f,0.0f,0.0f}};


    vkCmdPushConstants(cmd_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ConstantRanges), &push);
}

void ObjectManager::update_shader_data(std::chrono::microseconds dt, u_int32_t target_index){
    void * data_ptr;
    if( vkMapMemory(this->device.get_device(), this->vertex_buffer_memory[target_index], 0, sizeof(Vertex)*this->vertex_count, 0, &data_ptr) != VK_SUCCESS)
        throw std::runtime_error("Failed to Map Memory");
    memcpy(data_ptr, this->vertices.data(), sizeof(Vertex)*this->vertices.size());
    vkUnmapMemory(this->device.get_device(), this->vertex_buffer_memory[target_index]);
    if constexpr (debug)
        std::cout << "Copied " << sizeof(Vertex)*this->vertices.size() << " bytes" << std::endl;
}

void ObjectManager::bind_vertex_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index){
    VkDeviceSize offset{0};
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &this->vertex_buffers[index], &offset);
}


} // namespace velora
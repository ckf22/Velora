#include "descriptors.hpp"

#include <iostream>
#include <stdexcept>

namespace velora{

DescriptorPool::DescriptorPool(Device& _device, u_int32_t _max_sets) : device{_device}, max_sets{_max_sets} {
    VkDescriptorPoolSize pool_size{
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .descriptorCount = 10,
    };

    VkDescriptorPoolCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = _max_sets,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size
    };

    if( vkCreateDescriptorPool(this->device.get_device(), &ci, VK_NULL_HANDLE, &this->descriptor_pool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Descriptor Pool");

    if constexpr (debug)
        std::cout << "Descriptor Pool Created" << std::endl;

    this->create_temp();
}

DescriptorPool::~DescriptorPool(){
    vkDestroyDescriptorSetLayout(this->device.get_device(), this->layout, nullptr);
    vkDestroyDescriptorPool(this->device.get_device(), this->descriptor_pool, nullptr);
}

void DescriptorPool::create_temp(){
    VkDescriptorSetLayoutBinding ds_binding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };
    VkDescriptorSetLayoutCreateInfo layout_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = 0,
        .bindingCount = 1,
        .pBindings = &ds_binding,
    };

    if( vkCreateDescriptorSetLayout(this->device.get_device(), &layout_ci, VK_NULL_HANDLE, &this->layout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Descriptor Set Layout");

    VkDescriptorSetAllocateInfo alloc_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = this->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &this->layout,
    };
    if( vkAllocateDescriptorSets(this->device.get_device(), &alloc_ci, &this->set) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Descriptor Set Layout");


}

void DescriptorPool::add_b(MyBuffer& buffer){
    VkDescriptorBufferInfo buffer_info{
        .buffer = buffer.get_buffer(),
        .offset = 0,
        .range = buffer.get_size(),
    };
    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = this->set,
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .pBufferInfo = &buffer_info,
    };
    vkUpdateDescriptorSets(this->device.get_device(), 1, &write, 0, nullptr);
}


DescriptorSet::DescriptorSet(Device& _device, DescriptorPool& _pool) : device{_device}, pool{_pool} {}
DescriptorSet::~DescriptorSet(){}


DescriptorManager::DescriptorManager(Device& _device) : device{_device} {}
DescriptorManager::~DescriptorManager(){}
void DescriptorManager::add_descriptor_set(){}
void DescriptorManager::create_ressources(){}


}
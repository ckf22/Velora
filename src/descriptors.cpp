#include "descriptors.hpp"

#include <iostream>
#include <stdexcept>

namespace velora{

Descriptors::Descriptors(Device& _device, const u_int32_t _descriptor_set_count) : device{_device}, descriptor_set_count{_descriptor_set_count} {}
Descriptors::~Descriptors(){
    vkDestroyDescriptorSetLayout(this->device.get_device(), this->layout, nullptr);
    vkDestroyDescriptorPool(this->device.get_device(), this->pool, nullptr);
}

void Descriptors::add_binding(VkDescriptorSetLayoutBinding binding){
    if( this->ressources_created )
        throw std::logic_error("Attempt to add binding to descriptors failed: Ressources have already been created");

    this->bindings.push_back(binding);
    this->pool_size.push_back({binding.descriptorType, binding.descriptorCount*this->descriptor_set_count});
}

VkDescriptorSetLayout& Descriptors::generate_layout(){
    VkDescriptorSetLayoutCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<u_int32_t>(this->bindings.size()),
        .pBindings = this->bindings.data(),
    };

    if( vkCreateDescriptorSetLayout(this->device.get_device(), &ci, VK_NULL_HANDLE, &this->layout) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Descriptor Set Layout");

    return this->layout;
}


void Descriptors::generate_sets(){
    VkDescriptorPoolCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = this->descriptor_set_count,
        .poolSizeCount = static_cast<u_int32_t>(this->pool_size.size()),
        .pPoolSizes = this->pool_size.data(),
    };

    if( vkCreateDescriptorPool(this->device.get_device(), &ci, VK_NULL_HANDLE, &this->pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Descriptor Pool");


    VkDescriptorSetAllocateInfo alloc{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = this->pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &this->layout,
    };

    this->sets.resize(this->descriptor_set_count);
    for(int i = 0; i < this->descriptor_set_count; ++i)
        if( vkAllocateDescriptorSets(this->device.get_device(), &alloc, &this->sets[i]) != VK_SUCCESS )
            throw std::runtime_error("Failed to allocate Descriptor Set");
}

void Descriptors::allocate_descriptor(VkWriteDescriptorSet write, u_int32_t index){
    write.dstSet = this->sets.at(index);
    vkUpdateDescriptorSets(this->device.get_device(), 1, &write, 0, nullptr);
}


} // namespace velora
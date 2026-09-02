#include "descriptors.hpp"

#include <iostream>
#include <stdexcept>

namespace velora{

Descriptors::Descriptors(Device& _device, const u_int32_t _descriptor_set_count) : device{_device}, descriptor_set_count{_descriptor_set_count} {}
Descriptors::~Descriptors(){
    vkDestroyDescriptorSetLayout(this->device.get_device(), this->layout, nullptr);
    vkDestroyDescriptorPool(this->device.get_device(), this->pool, nullptr);
}

void Descriptors::bind_descriptor_set(VkCommandBuffer& cmd_buffer, VkPipelineLayout& layout, u_int32_t index){
    std::vector<u_int32_t> offsets(this->dynamic_descriptor_count, 0);
    vkCmdBindDescriptorSets(
        cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
        &this->sets[index], this->dynamic_descriptor_count, offsets.data()
    );
}


void Descriptors::add_binding(VkDescriptorSetLayoutBinding binding){
    if( this->ressources_creation_stage != 0 )
        throw std::logic_error("Attempt to add binding to descriptors failed: Ressources have already been created");

    if( binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC || binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC )
        this->dynamic_descriptor_count += 1;

    this->bindings.push_back(binding);
    this->pool_size.push_back({binding.descriptorType, binding.descriptorCount*this->descriptor_set_count});
}

VkDescriptorSetLayout& Descriptors::generate_layout(){
    if( this->ressources_creation_stage != 0 )
        throw std::logic_error("Attempted generation of Descriptor Layout failed: Layout has already been created");

    VkDescriptorSetLayoutCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<u_int32_t>(this->bindings.size()),
        .pBindings = this->bindings.data(),
    };

    if( vkCreateDescriptorSetLayout(this->device.get_device(), &ci, VK_NULL_HANDLE, &this->layout) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Descriptor Set Layout");

    this->ressources_creation_stage = 1;

    if constexpr(debug)
        std::cout << "Descriptor Set Layout created" << std::endl;

    this->generate_pool();

    return this->layout;
}

void Descriptors::generate_pool(){
    if( this->ressources_creation_stage != 1 )
        throw std::logic_error("Attempted generation of Descriptor Pool failed: Pool has already been created");


    VkDescriptorPoolCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = this->descriptor_set_count,
        .poolSizeCount = static_cast<u_int32_t>(this->pool_size.size()),
        .pPoolSizes = this->pool_size.data(),
    };

    if( vkCreateDescriptorPool(this->device.get_device(), &ci, VK_NULL_HANDLE, &this->pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Descriptor Pool");

    this->ressources_creation_stage = 2;
}

void Descriptors::generate_sets(){
    if( this->ressources_creation_stage != 2 )
        throw std::logic_error("Attempted generation of Descriptor Sets failed: Sets has already been created");

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

    this->ressources_creation_stage = 3;

    if constexpr(debug)
        std::cout << "Descriptor Sets allocated" << std::endl;
}

void Descriptors::allocate_descriptor(VkWriteDescriptorSet write, u_int32_t index){
    if( this->ressources_creation_stage != 3 )
        throw std::logic_error("Attempted allocation of Descriptor failed: Sets have not been created");

    write.dstSet = this->sets.at(index);
    vkUpdateDescriptorSets(this->device.get_device(), 1, &write, 0, nullptr);
}


} // namespace velora
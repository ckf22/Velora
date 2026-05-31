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


DescriptorManager::DescriptorManager(Device& _device) : device{_device} {}
DescriptorManager::~DescriptorManager(){
    if( this->ressources_created_flag == true )
        for(auto& it : this->layouts)
            vkDestroyDescriptorSetLayout(this->device.get_device(), it.layout, nullptr);
}

std::vector<VkDescriptorSetLayout> DescriptorManager::get_layout_vector(){
    std::vector<VkDescriptorSetLayout> ret(this->layouts.size());
    for(int i = 0; i < this->layouts.size(); ++i)
        ret[i] = this->layouts[i].layout;
    return ret;
}

void DescriptorManager::allocate_buffer_descriptor(int target_set, MyBuffer& buffer, u_int32_t shader_binding, u_int32_t buffer_offset){
    if( this->ressources_created_flag == false )
        throw std::runtime_error("Ressources have not been created");

        VkDescriptorBufferInfo buffer_info{
        .buffer = buffer.get_buffer(),
        .offset = buffer_offset,
        .range = buffer.get_size()
    };

    VkWriteDescriptorSet write_data{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = this->sets[target_set].set,
        .dstBinding = shader_binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .pBufferInfo = &buffer_info,
    };

    vkUpdateDescriptorSets(this->device.get_device(), 1, &write_data, 0, nullptr);
}


void DescriptorManager::create_ressources(){
    if( this->ressources_created_flag == true )
        throw std::runtime_error("Ressources have already been created");

    u_int32_t count = 0;
    for(auto& it : this->set_create_data) count += it.count;
    this->pool = std::make_unique<DescriptorPool>(this->device, count);

    for(auto& it : this->set_create_data){
        this->sets.push_back(DescriptorSetData{
            .alloc_info = VkDescriptorSetAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = this->pool->descriptor_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &this->layouts[it.layout_id].layout,
            }
        });

        if( vkAllocateDescriptorSets(this->device.get_device(), &this->sets[this->sets.size()-1].alloc_info, &this->sets[this->sets.size()-1].set) != VK_SUCCESS )
            throw std::runtime_error(std::string("Failed to create Descriptor Set from ")+std::to_string(it.layout_id));
    }

    this->pool_sizes.clear();
    this->set_create_data.clear();

    this->ressources_created_flag = true;
}

int DescriptorManager::add_layout(std::vector<VkDescriptorSetLayoutBinding> binding_infos, VkDescriptorSetLayoutCreateFlags flags){
    if( this->ressources_created_flag == true )
        throw std::runtime_error("Ressources have already been created");

    this->layouts.push_back( DescriptorSetLayoutData{
        .ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .flags = flags,
            .bindingCount = static_cast<u_int32_t>(binding_infos.size()),
            .pBindings = binding_infos.data(),
        }
    });

    if( vkCreateDescriptorSetLayout(this->device.get_device(), &this->layouts[this->layouts.size()-1].ci, nullptr, &this->layouts[this->layouts.size()-1].layout ) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Descriptor Set Layout");

    return this->layouts.size() - 1;
}

int DescriptorManager::add_descriptor_set(int layout_id, u_int32_t set_count){
    if( this->ressources_created_flag == true )
        throw std::runtime_error("Ressources have already been created");


    for(int i = 0; i < set_count; ++i)
        this->set_create_data.push_back(DescriptorSetCreateData{.layout_id = layout_id, .count = 1});
    
    auto a = this->set_create_data.size() - set_count;  // returns the index of the first set of this call
    std::cout << "Descriptor Set registered" << std::endl;
    return a;
}

void DescriptorManager::add_descriptor(VkDescriptorType descriptor_type, u_int32_t descriptor_count){
    if( this->ressources_created_flag == true )
        throw std::runtime_error("Ressources have already been created");

    for(auto& it : this->pool_sizes){
        if( it.type == descriptor_type ){
            it.descriptorCount += descriptor_count;
            return;
        }
    }

    this->pool_sizes.push_back( VkDescriptorPoolSize{.type = descriptor_type, .descriptorCount = descriptor_count} );    
}


} // namespace velora
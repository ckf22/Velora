#include "texture-manager.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <filesystem>


namespace velora{

TextureManager::TextureManager(std::shared_ptr<Device> _device, const u_int32_t _frame_count, const VkCommandPool& cmd_pool, std::string filename) 
 : device{_device}, frame_count{_frame_count} {
    this->load_directory(filename, cmd_pool);

    this->generare_sampler();

    this->generate_descriptor_ressources();
}

TextureManager::~TextureManager(){
    vkDestroyDescriptorSetLayout(this->device->get_device(), this->layout, nullptr);
    vkDestroyDescriptorPool(this->device->get_device(), this->pool, nullptr);
    vkDestroySampler(this->device->get_device(), this->linear_sampler, nullptr);
}


void TextureManager::bind(const VkCommandBuffer& cmd_buffer, const VkPipelineLayout& layout, u_int32_t index){
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1, &this->sets.at(index), 0, nullptr);
}

void TextureManager::load_directory(std::string directory, const VkCommandPool& pool){

    std::filesystem::directory_iterator iterator{directory};

    for(const auto& it : iterator){
        if( it.exists() && !it.is_directory() )
            this->textures.push_back(
                std::make_unique<Texture>(this->device, pool, it.path())
            );
    }
}

void TextureManager::generare_sampler(){

    VkSamplerCreateInfo linear_sampler_ci{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 8.f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    linear_sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    linear_sampler_ci.mipLodBias = 0.0f;
    linear_sampler_ci.minLod = 0.0f;
    linear_sampler_ci.maxLod = 0.0f;

    if( vkCreateSampler(this->device->get_device(), &linear_sampler_ci, VK_NULL_HANDLE, &this->linear_sampler) )
        throw std::runtime_error("Failed to Create Sampler");

}

void TextureManager::generate_descriptor_ressources(){
    u_int32_t tex_count = this->textures.size();
    std::vector<VkDescriptorSetLayoutBinding> bindings{
        VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = tex_count,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        }
    };

    VkDescriptorBindingFlags binding_flags = 
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 1,
        .pBindingFlags = &binding_flags
    };

    VkDescriptorSetLayoutCreateInfo layout_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &binding_flags_ci,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
        .bindingCount = static_cast<u_int32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    if( vkCreateDescriptorSetLayout(this->device->get_device(), &layout_ci, VK_NULL_HANDLE, &this->layout) != VK_SUCCESS )
        throw std::runtime_error("Failed to generate texture Descriptor Set Layout");

    
    // Creating Descriptor Pool
    VkDescriptorPoolSize pool_size{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = tex_count * this->frame_count,
    };

    VkDescriptorPoolCreateInfo pool_ci{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = this->frame_count,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size
    };

    if( vkCreateDescriptorPool(this->device->get_device(), &pool_ci, nullptr, &this->pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Pool");


    // Sets
    u_int32_t countcount = tex_count;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = 1,
        .pDescriptorCounts = &countcount,
    };

    VkDescriptorSetAllocateInfo set_alloc_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &variable_count,
        .descriptorPool = this->pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &this->layout,
    };

        
    VkDescriptorSet buffer;
    for(int i = 0; i < this->frame_count; ++i){
        if( vkAllocateDescriptorSets(this->device->get_device(), &set_alloc_info, &buffer) != VK_SUCCESS )
            throw std::runtime_error("Failed to allocate Descriptor Set");

        this->sets.push_back(buffer);
    }


    // Allocating descriptors
    VkDescriptorImageInfo image_info{
        .sampler = this->linear_sampler,
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &image_info,
    };

    for(auto& it : this->textures){
        image_info.imageView = it->get_image_view();

        for(int i = 0; i < this->frame_count; ++i){
            write.dstSet = this->sets[i];

            vkUpdateDescriptorSets(this->device->get_device(), 1, &write, 0, nullptr);
        }

        write.dstArrayElement += 1;
    }

}




}
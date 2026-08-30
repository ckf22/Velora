#include "textures.hpp"

#include "buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"

#include <iostream>
#include <stdexcept>
#include <chrono>


namespace velora{

TextureManager::TextureManager(Device& _device, Descriptors& _descriptors, VkCommandPool& cmd_pool, std::string filename) 
 : device{_device}, descriptors{_descriptors} {
    this->load_texture(cmd_pool, filename);
    this->register_descriptors();
}

TextureManager::~TextureManager(){
    vkDestroyImageView(this->device.get_device(), this->image_view, VK_NULL_HANDLE);
    vkDestroyImage(this->device.get_device(), this->texture, VK_NULL_HANDLE);
    vkFreeMemory(this->device.get_device(), this->texture_ram, VK_NULL_HANDLE);
    vkDestroySampler(this->device.get_device(), this->sampler, VK_NULL_HANDLE);
}

void TextureManager::load_texture(VkCommandPool& cmd_pool, std::string filename){
    auto t0 = std::chrono::high_resolution_clock::now();

    int width, height, channels;

    stbi_uc * pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if( !pixels )
        throw std::runtime_error(std::string("Failed to load image from file: \'")+filename+'\'');

    MyBuffer staging(
        this->device, 4, width*height,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    );
    memcpy( staging.map(), (void*)pixels, staging.get_size() );
    staging.unmap();

    stbi_image_free(pixels);

    VkExtent3D extent{
        .width = static_cast<u_int32_t>(width),
        .height = static_cast<u_int32_t>(height),
        .depth = 1 // number of textures
    };

    VkImageCreateInfo image_ci{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    if( vkCreateImage(this->device.get_device(), &image_ci, VK_NULL_HANDLE, &this->texture) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Texture Image");

    VkMemoryRequirements image_requirements;
    vkGetImageMemoryRequirements(this->device.get_device(), this->texture, &image_requirements);

    VkMemoryAllocateInfo image_alloc_ci{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = image_requirements.size,
        .memoryTypeIndex = this->device.find_memory_type(image_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    if( vkAllocateMemory(this->device.get_device(), &image_alloc_ci, VK_NULL_HANDLE, &this->texture_ram) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate RAM");

    if( vkBindImageMemory(this->device.get_device(), this->texture, this->texture_ram, 0) != VK_SUCCESS)
        throw std::runtime_error("Failed to bind image RAM");


    VkImageViewCreateInfo image_view_ci{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = this->texture,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = image_ci.format,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = image_ci.mipLevels, .layerCount = 1 }
    };

    if( vkCreateImageView(this->device.get_device(), &image_view_ci, VK_NULL_HANDLE, &this->image_view) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Image View");


    VkCommandBuffer cmd_buffer;
    VkCommandBufferAllocateInfo cmd_buffer_alloc{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool,
        .commandBufferCount = 1
    };

    if( vkAllocateCommandBuffers(this->device.get_device(), &cmd_buffer_alloc, &cmd_buffer) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate one-time command buffer");

    VkCommandBufferBeginInfo cmd_buffer_begin{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    if( vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin) != VK_SUCCESS )
        throw std::runtime_error("Failed to begin Command Buffer");

    VkImageMemoryBarrier2 barrier_texture_image{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image = this->texture,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 },
    };
    VkDependencyInfo barrier_dependency{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier_texture_image,
    };
    vkCmdPipelineBarrier2(cmd_buffer, &barrier_dependency);

    VkBufferImageCopy copy_region{
        .bufferOffset = 0,
        .imageSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .layerCount = 1},
        .imageExtent = image_ci.extent,
    };
    vkCmdCopyBufferToImage(cmd_buffer, staging.get_buffer(), this->texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    VkImageMemoryBarrier2 read_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        .image = this->texture,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    };
    barrier_dependency.pImageMemoryBarriers = &read_barrier;
    vkCmdPipelineBarrier2(cmd_buffer, &barrier_dependency);

    if( vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS )
        throw std::runtime_error("Failed to end Command Buffer");

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer
    };

    if( vkQueueSubmit(this->device.get_queue(), 1, &submit_info, VK_NULL_HANDLE ) != VK_SUCCESS )
        throw std::runtime_error("Failed to submit Queue");

    if( vkQueueWaitIdle(this->device.get_queue()) != VK_SUCCESS )
        throw std::runtime_error("Failed to wait for queue");

    std::cout << "Image Loaded from file: \'" << filename << "\'("
              << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0).count()
              << "ms); " << static_cast<float>(staging.get_size())/1000000 << "MB RAM;" << std::endl;



    VkSamplerCreateInfo sampler_ci{
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
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.mipLodBias = 0.0f;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = 0.0f;

    if( vkCreateSampler(this->device.get_device(), &sampler_ci, VK_NULL_HANDLE, &this->sampler) )
        throw std::runtime_error("Failed to Create Sampler");
}

void TextureManager::allocate_descriptors(){
    VkDescriptorImageInfo image_info{
        .sampler = this->sampler,
        .imageView = this->image_view,
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 4,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &image_info,
    };

    for(u_int32_t i = 0; i < this->descriptors.get_set_count(); ++i)
        this->descriptors.allocate_descriptor(write, i);
}

void TextureManager::register_descriptors(){
    this->descriptors.add_binding(
        VkDescriptorSetLayoutBinding{
            .binding = 4,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        }
    );
}



}
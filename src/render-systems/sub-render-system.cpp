#include "sub-render-system.hpp"

namespace velora{

SubRenderSystem::~SubRenderSystem(){}

void SubRenderSystem::create_buffer_objects(u_int32_t buffer_count, u_int32_t vertex_bytes, u_int32_t index_bytes,
                                            u_int32_t ssbo_bytes, u_int32_t ubo_bytes){

    for(int i = 0; i < buffer_count; ++i){
        this->vertex_buffers.push_back( 
            std::make_unique<MyBuffer>(
                this->device, vertex_bytes, 1,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 
            )
        );
        this->index_buffers.push_back(
            std::make_unique<MyBuffer>(
                this->device, index_bytes, 1,
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            )
        );
        this->ssbo.push_back(
            std::make_unique<MyBuffer>(
                this->device, ssbo_bytes, 1,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            )
        );
        this->ssbo_staging.push_back(
            std::make_unique<MyBuffer>(
                this->device, ssbo_bytes, 1,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            ) 
        );
        this->ubo.push_back(
            std::make_unique<MyBuffer>(
                this->device, ubo_bytes, 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            )
        );
        this->ubo_staging.push_back(
            std::make_unique<MyBuffer>(
                this->device, ubo_bytes, 1,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            ) 
        );
    }
}

void SubRenderSystem::allocate_built_in_descriptors(u_int32_t ubo_binding, u_int32_t ssbo_binding){
    if(!this->descriptor_manager)
        throw std::logic_error("Descriptor Manager has not been created");

    VkDescriptorBufferInfo ubo_info{
        .buffer = nullptr,
        .offset = 0,
        .range = 0
    };
    VkWriteDescriptorSet write_ubo{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = ubo_binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .pBufferInfo = &ubo_info,
    };

    VkDescriptorBufferInfo ssbo_info{
        .buffer = nullptr,
        .offset = 0,
        .range = 0
    };
    VkWriteDescriptorSet write_ssbo{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = ssbo_binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .pBufferInfo = &ssbo_info,
    };

    for(int i = 0; i < this->frame_count; ++i){
        ubo_info.buffer = this->ubo.at(i)->get_buffer();
        ubo_info.range = this->ubo.at(i)->get_size();
        ssbo_info.buffer = this->ssbo.at(i)->get_buffer();
        ssbo_info.range = this->ssbo.at(i)->get_size();

        this->descriptor_manager->allocate_descriptor(write_ubo, i);
        this->descriptor_manager->allocate_descriptor(write_ssbo, i);
    }
}

void SubRenderSystem::register_built_in_descriptors(u_int32_t ubo_binding, u_int32_t ssbo_binding){
    if(!this->descriptor_manager)
        throw std::logic_error("Descriptor Manager has not been created");

    // UBO
    this->descriptor_manager->add_binding(
        VkDescriptorSetLayoutBinding{
            .binding = ubo_binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL
        }
    );

    // SSBO
    this->descriptor_manager->add_binding(
        VkDescriptorSetLayoutBinding{
            .binding = ssbo_binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL
        }
    );
}


void SubRenderSystem::internal_update_device_local_buffers(VkCommandBuffer& cmd_buffer, u_int32_t buffer_index){
    VkBufferCopy region{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = this->ubo.at(buffer_index)->get_size(),
    };

    // UBO
    vkCmdCopyBuffer(
        cmd_buffer, this->ubo_staging.at(buffer_index)->get_buffer(),
        this->ubo.at(buffer_index)->get_buffer(), 1, &region
    );

    region.size = this->ssbo.at(buffer_index)->get_size();

    // SSBO
    vkCmdCopyBuffer(
        cmd_buffer, this->ssbo_staging.at(buffer_index)->get_buffer(),
        this->ssbo.at(buffer_index)->get_buffer(), 1, &region
    );
}


}
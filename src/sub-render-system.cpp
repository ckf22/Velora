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


}
#include "buffer.hpp"

#include <iostream>
#include <stdexcept>

namespace velora{

MyBuffer::MyBuffer(
        Device& _device,
        u_int32_t _element_size,
        u_int32_t _element_count,
        VkMemoryPropertyFlags _property_flags,
        VkBufferUsageFlags _usage_flags
    ) : device{_device}, element_size{_element_size}, element_count{_element_count}, property_flags{_property_flags}, usage_flags{_usage_flags}
{
    this->create_buffer(_element_count * _element_size, _property_flags, _usage_flags);
}

MyBuffer::~MyBuffer(){
    if( this->mapped.has_value() )
        this->unmap();

    vkDestroyBuffer(this->device.get_device(), this->buffer, nullptr);
    vkFreeMemory(this->device.get_device(), this->ram, nullptr);
}

void * MyBuffer::map(VkDeviceSize bytes, VkDeviceSize offset){
    if( this->mapped.has_value() )
        return *this->mapped;
    
    void * dest;
    auto result = vkMapMemory(this->device.get_device(), this->ram, offset, bytes, 0, &dest);
    if( result != VK_SUCCESS ) throw std::runtime_error("Failed to map RAM");

    this->mapped = dest;

    return dest;
}

void MyBuffer::unmap(){
    if( this->mapped.has_value() ){
        vkUnmapMemory(this->device.get_device(), this->ram);
        this->mapped.reset();
    }
}

void MyBuffer::create_buffer(u_int32_t size, VkMemoryPropertyFlags property_flags, VkBufferUsageFlags usage_flags){
    VkBufferCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if( vkCreateBuffer(this->device.get_device(), &ci, VK_NULL_HANDLE, &this->buffer) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Buffer");

    VkMemoryRequirements ram_requirements;
    vkGetBufferMemoryRequirements(this->device.get_device(), this->buffer, &ram_requirements);

    VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = ram_requirements.size,
        .memoryTypeIndex = 
            this->device.find_memory_type(ram_requirements.memoryTypeBits, property_flags),
    };

    if( vkAllocateMemory(this->device.get_device(), &alloc_info, nullptr, &this->ram) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate RAM");

    if( vkBindBufferMemory(this->device.get_device(), this->buffer, this->ram, 0) != VK_SUCCESS )
        throw std::runtime_error("Failed to bind RAM");


}


} // namespace velora

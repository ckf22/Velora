#pragma once

#include "device.hpp"

namespace velora{

class MyBuffer{
  public:
    MyBuffer(
        Device& _device,
        u_int32_t _element_size,
        u_int32_t _element_count,
        VkMemoryPropertyFlags _property_flags,
        VkBufferUsageFlags _usage_flags
    );
    ~MyBuffer();

    MyBuffer(const MyBuffer&) = delete;
    MyBuffer& operator=(const MyBuffer&) = delete;

    void * map(VkDeviceSize bytes = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    const VkBuffer& get_buffer() const { return buffer; }
    const VkDeviceMemory& get_ram() const { return ram; }
    u_int32_t get_size() { return element_size * element_count; }
    VkMemoryPropertyFlags get_ram_flags() { return property_flags; }
    VkBufferUsageFlags get_usage_flags() { return usage_flags; }
  private:
    void create_buffer(
        u_int32_t size,
        VkMemoryPropertyFlags property_flags,
        VkBufferUsageFlags usage_flags
    );

    Device& device;

    u_int32_t element_size;
    u_int32_t element_count;

    VkMemoryPropertyFlags property_flags;
    VkBufferUsageFlags usage_flags;

    std::optional<void *> mapped;

    VkBuffer buffer;
    VkDeviceMemory ram;
};

}
#include "command-pool.hpp"

#include <stdexcept>

namespace velora{

CommandPool::CommandPool(Device& _device) : device{_device} {
    VkCommandPoolCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = _device.get_queue_family(),
    };

    if( vkCreateCommandPool(_device.get_device(), &ci, VK_NULL_HANDLE, &this->pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Command Pool");
}

CommandPool::~CommandPool(){
    vkDestroyCommandPool(this->device.get_device(), this->pool, nullptr);
}

}
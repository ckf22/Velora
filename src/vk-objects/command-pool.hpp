#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>

namespace velora{

class Device;

class CommandPool{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
  public:
    CommandPool(std::shared_ptr<Device> _device);
    ~CommandPool();

    CommandPool& operator=(const CommandPool&) = delete;
    CommandPool(CommandPool&) = delete;

    const VkCommandPool& get_pool() const { return pool; }
  private:
    std::shared_ptr<Device> device;

    VkCommandPool pool;
};

}
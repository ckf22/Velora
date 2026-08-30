#pragma once

#include "device.hpp"

namespace velora{

class CommandPool{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
  public:
    CommandPool(Device& _device);
    ~CommandPool();

    CommandPool& operator=(const CommandPool&) = delete;
    CommandPool(CommandPool&) = delete;

    const VkCommandPool& get_pool() const { return pool; }
  private:
    Device& device;

    VkCommandPool pool;
};

}
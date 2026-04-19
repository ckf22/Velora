#pragma once

#include "instance.hpp"
#include "device.hpp"

namespace velora{

class SwapChain{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
    const std::vector<VkPresentModeKHR> preffered_present_modes{VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR};
    const std::vector<VkFormat> preferred_depth_formats{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
  public:
    SwapChain(VkSurfaceKHR& _surface, Device& _device, unsigned int _width, unsigned int _height);
    ~SwapChain();

    void operator=(const SwapChain&) = delete;
    SwapChain(SwapChain&) = delete;

    VkFormat& get_image_format() { return image_format; }
    VkFormat& get_depth_format() { return depth_format; }

    void recreate_swapchain(unsigned int _width, unsigned int _height);
  private:
    void initialise_swapchain(Device& _device);

    VkFormat choose_format(VkSurfaceKHR& _surface, Device& device, VkSurfaceCapabilitiesKHR capabilities);
    VkPresentModeKHR choose_present_mode(VkSurfaceKHR& _surface, Device& device, VkSurfaceCapabilitiesKHR capabilities);
    VkFormat choose_depth_format(Device& _device);

    VkFormat depth_format;
    VkFormat image_format;

    VkSurfaceKHR& surface;
    VkDevice& device;

    VkExtent2D extent;
    VkSwapchainCreateInfoKHR swapchain_ci;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> images;
    std::vector<VkDeviceMemory> image_ram;
    std::vector<VkImageView> image_views;
    size_t current_index = 0;
};

}
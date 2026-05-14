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
    const std::vector<VkPresentModeKHR> preffered_present_modes{VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_LATEST_READY_EXT, VK_PRESENT_MODE_MAILBOX_KHR};
    const std::vector<VkFormat> preferred_depth_formats{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
  public:
    SwapChain(VkSurfaceKHR& _surface, Device& _device, unsigned int _width, unsigned int _height);
    ~SwapChain();

    void operator=(const SwapChain&) = delete;
    SwapChain(SwapChain&) = delete;

    VkSwapchainKHR& get_swapchain() { return swapchain; }
    VkFormat& get_image_format() { return image_format; }
    VkFormat& get_depth_format() { return depth_format; }
    VkImage& get_image(int i) { return images[i]; }
    VkImage& get_depth_image(int i) { return depth_images[i]; }
    VkImageView& get_image_view(int i) { return image_views[i]; }
    VkImageView& get_depth_image_view(int i) { return depth_image_views[i]; }
    VkExtent2D get_current_extent() { return extent; }
    VkFence& get_fence(int i) { return fences[i]; }
    size_t get_current_index() { return current_index; }
    int get_image_count() { return images.size(); }

    void wait_for_active_image_fence();
    void aquire_next_image(VkSemaphore& image_ready_semaphore);

    void recreate_swapchain(unsigned int _width, unsigned int _height);
  private:
    void initialise_swapchain();

    void delete_objects();

    VkFormat choose_format(VkSurfaceKHR& _surface, VkSurfaceCapabilitiesKHR capabilities);
    VkPresentModeKHR choose_present_mode(VkSurfaceKHR& _surface, VkSurfaceCapabilitiesKHR capabilities);
    VkFormat choose_depth_format();

    VkFormat depth_format;
    VkFormat image_format;

    VkSurfaceKHR& surface;
    Device& device;

    VkExtent2D extent;
    VkSwapchainCreateInfoKHR swapchain_ci;

    VkSwapchainKHR swapchain;

    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;

    std::vector<VkImage> depth_images;
    std::vector<VkImageView> depth_image_views;
    std::vector<VkDeviceMemory> depth_image_ram;

    std::vector<VkFence> fences;
    size_t current_index = 0;
};

}
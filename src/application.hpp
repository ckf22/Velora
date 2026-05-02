#pragma once

#include "window.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

#include <vector>

namespace velora{
    
class Application{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
  public:
    Application();
    ~Application();

    void run(float frame_time_ms = 16.67);
  private:
    void create_command_buffers(u_int32_t queue_family_index);
    void create_semaphores();

    void record_command_buffers();
    void submit_command_buffers(VkCommandBuffer& cmd_buffer, VkSemaphore& image_aquired_semaphore, VkFence& fence);

    void present_image();

    static constexpr int WIDTH = 800, HEIGHT = 600;

    std::vector<VkSemaphore> image_ready_semaphores;
    std::vector<VkSemaphore> image_aquired_semaphores;

    VkCommandPool command_pool{};
    std::vector<VkCommandBuffer> command_buffers;

    Window window{WIDTH, HEIGHT, "Vulkan Window"};
    Device device{window.get_window()};
    SwapChain swapchain{device.get_surface(), device, WIDTH, HEIGHT};
    Pipeline pipeline{device, "./shaders/constants-shader.vert.spv", "./shaders/constants-shader.frag.spv", {WIDTH,HEIGHT}, &swapchain.get_image_format(), swapchain.get_depth_format()};
};

}
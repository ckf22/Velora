#pragma once

#include "window.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "render-system.hpp"

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

    void run(float fps = 60);
    void resize(u_int32_t width, u_int32_t height);
  private:
    void resize(); // resizes to current window dimensions
    void create_command_buffers(u_int32_t queue_family_index);
    void create_semaphores();

    void record_command_buffers();
    void submit_command_buffers(VkSemaphore& image_aquired_semaphore);

    void present_image();

    static constexpr int WIDTH = 800, HEIGHT = 600;

    std::vector<VkSemaphore> image_ready_semaphores;
    std::vector<VkSemaphore> image_aquired_semaphores;

    VkCommandPool command_pool{};
    std::vector<VkCommandBuffer> command_buffers;

    Window window{"Vulkan Window", WIDTH, HEIGHT};
    Device device{window.get_window()};
    SwapChain swapchain{device.get_surface(), device, WIDTH, HEIGHT};
    Pipeline pipeline{device, "./shaders/simple-3d-shader.vert.spv", "./shaders/simple-3d-shader.frag.spv", {WIDTH,HEIGHT}, &swapchain.get_image_format(), swapchain.get_depth_format()};

    RenderSystem render_system{device, static_cast<u_int32_t>(swapchain.get_image_count()), WIDTH, HEIGHT};

};

}
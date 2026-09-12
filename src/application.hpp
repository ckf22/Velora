#pragma once

#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "movement-controller.hpp"
#include "textures.hpp"
#include "command-pool.hpp"
#include "vertex-render-system.hpp"

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
    void apply_resize_to_camera(VkExtent2D extent);

    void create_command_buffers(u_int32_t queue_family_index);
    void create_semaphores();

    void record_command_buffers();
    void submit_command_buffers(VkSemaphore& image_aquired_semaphore);

    void present_image();

    static constexpr int WIDTH = 800, HEIGHT = 600;

    std::vector<VkSemaphore> image_ready_semaphores;
    std::vector<VkSemaphore> image_aquired_semaphores;

    std::vector<VkCommandBuffer> command_buffers;

    Camera camera{};

    Window window{"Vulkan Window", WIDTH, HEIGHT};
    MovementController movement_controller{window};

    Device device{window.get_window()};
    CommandPool command_pool{device};

    SwapChain swapchain{device.get_surface(), device, WIDTH, HEIGHT};

    std::unique_ptr<VertexRenderSystem> vertex_render_system;
};

}
#pragma once

#include "window.hpp"
#include "swapchain.hpp"
#include "movement-controller.hpp"
#include "command-pool.hpp"
#include "vertex-render-system.hpp"
#include "point-light-system.hpp"

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
    // resizes to current window dimensions
    void resize();
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
    MovementController movement_controller{};

    std::shared_ptr<Device> device = std::make_shared<Device>(window.get_window());

    CommandPool command_pool{device};

    SwapChain swapchain{device->get_surface(), device, WIDTH, HEIGHT};

    std::shared_ptr<PointLightSystem> point_light_system = std::make_shared<PointLightSystem>(device, 100, this->swapchain.get_image_count());

    std::unique_ptr<VertexRenderSystem> vertex_render_system;
};

}
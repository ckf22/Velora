#pragma once

#include "window.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "descriptors.hpp"
#include "render-system.hpp"
#include "movement-controller.hpp"
#include "textures.hpp"
#include "command-pool.hpp"

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

    std::vector<VkCommandBuffer> command_buffers;


    Window window{"Vulkan Window", WIDTH, HEIGHT};
    MovementController movement_controller{window};

    Device device{window.get_window()};
    CommandPool command_pool{device};

    SwapChain swapchain{device.get_surface(), device, WIDTH, HEIGHT};
    Descriptors descriptor_manager{device, swapchain.get_image_count()};

    RenderSystem render_system{device, movement_controller, descriptor_manager, static_cast<u_int32_t>(swapchain.get_image_count()), WIDTH, HEIGHT};
    // dont switch this defintion with the one above
    TextureManager textures{device, descriptor_manager, const_cast<VkCommandPool&>(command_pool.get_pool()), "./assets/brick-texture.png"};

    Pipeline pipeline{
      device, std::vector{this->descriptor_manager.generate_layout()},
      "./shaders/ssbo-3d-shader.vert.spv", "./shaders/texture-shader.frag.spv",
      VkExtent2D{WIDTH,HEIGHT}, &swapchain.get_image_format(), swapchain.get_depth_format()
    };

};

}
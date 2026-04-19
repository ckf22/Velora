#pragma once

#include "window.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

namespace velora{
    
class Application{
  public:
    Application();
    ~Application();

    void run();
  private:
    static constexpr int WIDTH = 800, HEIGHT = 600;

    Window window{WIDTH, HEIGHT, "Vulkan Window"};
    Device device{window.get_window()};
    SwapChain swapchain{device.get_surface(), device, WIDTH, HEIGHT};
    Pipeline pipelines{device, "./shaders/constants-shader.vert.spv", "./shaders/constants-shader.frag.spv", {WIDTH,HEIGHT}, &swapchain.get_image_format(), swapchain.get_depth_format()};
};

}
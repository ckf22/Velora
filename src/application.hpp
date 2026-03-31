#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swapchain.hpp"

#include <memory>
#include <vector>

namespace velora{
    
class Application{
  public:
    Application();
    ~Application();

    Application(const Application &) = delete;
    void operator=(const Application &) = delete;

    void run();
  private:
    static constexpr int WIDTH = 800, HEIGHT = 600;

    void create_pipeline_layout();
    void create_pipeline();
    void create_command_buffers();

    void draw_frame();

    Window window{WIDTH, HEIGHT, "Vulkan Window"};
    Device device{window};
    SwapChain swapchain{device, window.get_dimensions()};
    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout;
    std::vector<VkCommandBuffer> command_buffers;

};

}; // namespace velaro
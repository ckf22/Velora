#pragma once

#include "window.hpp"
#include "device.hpp"

#include <memory>

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
};
};
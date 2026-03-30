#pragma once

#include "window.hpp"

namespace velora{
    
class Application{
  public:
    Application();
    ~Application();

    void run();
  private:
    static constexpr int WIDTH = 800, HEIGHT = 600;

    Window window{WIDTH, HEIGHT, "Vulkan Window"};
};
};
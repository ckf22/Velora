#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <string>

namespace velora{

class Window{
  public:
    Window(unsigned int w, unsigned int h, std::string name);
    ~Window();

    Window(const Window& w) = delete;
    Window& operator=(const Window& w) = delete;

    bool should_close();
    GLFWwindow& get_window() { return *window; }
  private:
    void create_window();

    GLFWwindow * window;

    const unsigned int width;
    const unsigned int height;
    const std::string name;
};

};
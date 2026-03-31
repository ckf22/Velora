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
    VkExtent2D get_dimensions(){ return {this->width, this->height}; }

    void create_window_surface(VkInstance instance, VkSurfaceKHR * surface);
  private:
    void create_window();

    GLFWwindow * window;

    const unsigned int width;
    const unsigned int height;
    const std::string name;
};

};
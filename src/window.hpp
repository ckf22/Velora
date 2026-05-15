#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <string>

namespace velora{

class Window{
  public:
    Window(std::string _window_name, unsigned int w = 800, unsigned int h = 600);
    ~Window();

    Window(const Window& w) = delete;
    Window& operator=(const Window& w) = delete;


    VkExtent2D get_window_extent();
    bool should_close();
    GLFWwindow& get_window() { return *window; }
    bool was_window_resized() { return window_resize_flag; }
    void reset_window_resized_flag() { window_resize_flag = false; }
  private:
    void create_window();

    static void resized_callback(GLFWwindow * window, int width, int height);

    const std::string window_name;
    int width = 800, height = 600;
    bool window_resize_flag = false;
    GLFWwindow * window;
};

}
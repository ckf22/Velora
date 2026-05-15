#include "window.hpp"
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

namespace velora {

Window::Window(std::string _window_name, unsigned int w, unsigned int h) : width{static_cast<int>(w)}, height{static_cast<int>(h)}, window_name{_window_name} {
    this->create_window();
}

Window::~Window(){
    glfwDestroyWindow(this->window);
    glfwTerminate();
}

bool Window::should_close(){
    return glfwWindowShouldClose(this->window);
}

VkExtent2D Window::get_window_extent(){
    return {static_cast<u_int32_t>(this->width), static_cast<u_int32_t>(this->height)};
}


void Window::create_window(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(this->width, this->height, this->window_name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(this->window, this);
    glfwSetFramebufferSizeCallback(this->window, resized_callback);
}

void Window::resized_callback(GLFWwindow * window, int width, int height){
    Window* user_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    user_window->width = width;
    user_window->height = height;
    user_window->window_resize_flag = true;
}

}  // namespace velora
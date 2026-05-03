#include "window.hpp"
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

namespace velora {

Window::Window(unsigned int w, unsigned int h, std::string window_name) : width(w), height(h), name(window_name) {
    this->create_window();
}

Window::~Window(){
    glfwDestroyWindow(this->window);
    glfwTerminate();
}

bool Window::should_close(){
    return glfwWindowShouldClose(this->window);
}

void Window::create_window(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(this->width, this->height, this->name.c_str(), nullptr, nullptr);
}

}  // namespace velora
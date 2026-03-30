#include "application.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>

namespace velora {
Application::Application(){}
Application::~Application(){}

void Application::run(){
    while(!this->window.should_close()){
        glfwPollEvents();
    }
}
}

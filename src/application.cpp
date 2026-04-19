#include "application.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <chrono>

namespace velora {
Application::Application(){}

Application::~Application(){}

void Application::run(){
    const float frame_time_ms = 100/6;
    auto t0 = std::chrono::high_resolution_clock::now();
    while(!this->window.should_close()){
        glfwPollEvents();
        // resetting time here to include 'should_close()' function call
        t0 = std::chrono::high_resolution_clock::now();
    }
}
}

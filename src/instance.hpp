#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#define DEBUG

namespace velora{

class Instance{
  public:
    Instance();
    ~Instance();

    VkInstance& get_instance() { return instance; }
  private:
    std::vector<const char*> get_required_extensions();
    void create_instance(VkApplicationInfo& app_info);

    #ifdef DEBUG
    static constexpr bool debug = true;
    #endif
    #ifndef DEBUG
    static constexpr bool debug = false;
    #endif

    VkInstance instance;
};

};
#pragma once

#define DEBUG

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace velora{

class Instance{
  public:
    Instance();
    ~Instance();

    Instance(Instance&) = delete;
    void operator=(const Instance&) = delete;

    VkInstance& get_instance() { return instance; }
    std::vector<const char*>& get_required_extensions(){ return this->required_extensions; };
  private:
    void initialise_required_extensions();
    void check_glfw_extension_support(std::vector<const char*> required);
    void create_instance(VkApplicationInfo& app_info);

    bool check_validation_layer_support();

    #ifdef DEBUG
    static constexpr bool debug = true;
    const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
    #else
    static constexpr bool debug = false;
    #endif

    std::vector<const char*> required_extensions = {};
    VkInstance instance;
};

};
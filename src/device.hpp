#pragma once

#include "instance.hpp"
#include "window.hpp"

#include <memory>
#include <vector>
#include <optional>

namespace velora
{

class Device{
  public:
    Device(GLFWwindow& window);
    ~Device();

    Device(Device&) = delete;
    void operator=(const Device&) = delete;

    VkDevice& get_device() { return logical_device; }
  private:
    #ifndef DEBUG
    static constexpr bool debug = false;
    #endif
    #ifdef DEBUG
    static constexpr bool debug = true;
    #endif

    void create_window_surface(GLFWwindow& window);
    void select_device();
    void create_logical_device();

    bool is_device_suitable(VkPhysicalDevice _device);
    float rate_device(VkPhysicalDevice _device);

    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkSurfaceKHR surface;
    VkSurfaceCapabilities2KHR surface_capabilities{};

    std::optional<std::vector<const char*>> required_extensions;
    Instance instance;
};

} // namespace velora

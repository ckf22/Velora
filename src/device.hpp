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

    u_int32_t find_memory_type(u_int32_t filter, VkMemoryPropertyFlags flags);

    VkDevice& get_device() { return logical_device; }
    VkPhysicalDevice& get_physical_device() { return physical_device; }
    VkSurfaceKHR& get_surface() { return surface; }
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

    VkSurfaceCapabilities2KHR surface_capabilities;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info;

    std::optional<std::vector<const char*>> required_extensions;
    Instance instance;
};

} // namespace velora

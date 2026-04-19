#include "device.hpp"

#include <stdexcept>
#include <iostream>
#include <optional>

namespace velora
{

Device::Device(GLFWwindow& window){
    this->create_window_surface(window);
    this->select_device();
    this->create_logical_device();
}

Device::~Device(){
    vkDestroySurfaceKHR(this->instance.get_instance(), this->surface, nullptr);
    vkDestroyDevice(this->logical_device, nullptr);
}

u_int32_t Device::find_memory_type(u_int32_t filter, VkMemoryPropertyFlags flags){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(this->physical_device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Device::create_window_surface(GLFWwindow& window){
    if( glfwCreateWindowSurface(this->instance.get_instance(), &window, nullptr, &this->surface) != VK_SUCCESS ){
        throw std::runtime_error("failed to create window surface");
    }

    //vkGetPhysicalDeviceSurfaceCapabilities2KHR(this->physical_device, &this->surface_info, &this->surface_capabilities);

    if constexpr (debug)
        std::cout << "Window surface created" << std::endl;
}

void Device::select_device(){
    uint32_t count{ 0 };
    if( vkEnumeratePhysicalDevices(this->instance.get_instance(), &count, nullptr) != VK_SUCCESS )
        throw std::runtime_error("Failed to enumerate physical devices");

    if( count == 0 )
        throw std::runtime_error("Failed to aquire GPU with Vulkan support");

    std::vector<VkPhysicalDevice> devices(count);
    if( vkEnumeratePhysicalDevices(this->instance.get_instance(), &count, devices.data()) != VK_SUCCESS )
        throw std::runtime_error("Failed to list physical devices");

    this->physical_device = devices[0];

    std::optional<VkPhysicalDevice> current;
    float max = -1;
    float buffer;

    if constexpr (debug)
        std::cout << "Avalible Graphic Cards" << std::endl;

    for(auto it : devices){
        if constexpr (debug){
            VkPhysicalDeviceProperties2 properties{ .sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
            vkGetPhysicalDeviceProperties2(it, &properties);
            std::cout << '\t' << properties.properties.deviceName << std::endl;
        }

        buffer = this->rate_device(it);
        if( this->is_device_suitable(it) && buffer > max ){
            current = it;
            max = buffer;
        }
    }

    if( !current.has_value() )
        throw std::runtime_error("Failed to find suitable device");

    this->physical_device = *current;

    if constexpr (debug){
        VkPhysicalDeviceProperties2 properties{ .sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        vkGetPhysicalDeviceProperties2(this->physical_device, &properties);
        std::cout << "Selected Graphics Card\n\t" << properties.properties.deviceName << std::endl;
    }
}

void Device::create_logical_device(){
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queues{queue_count};
    vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &queue_count, queues.data());

    std::optional<u_int32_t> queue_index;
    for(int i = 0; i < queues.size(); ++i){
        if( queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ){
            queue_index = static_cast<uint32_t>(i);
            if constexpr (debug){
                std::cout << "Queue found" << std::endl;
            }
            break;
        }
    }

    if( !queue_index.has_value() )
        throw std::runtime_error("Failed to find suitable QueueFamilies");

    const float qfpriorities{ 1.0f };

    VkDeviceQueueCreateInfo queue_ci{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = *queue_index,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };

    VkPhysicalDeviceVulkan12Features vk_1_2_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true
    };
    VkPhysicalDeviceVulkan13Features vulkan_1_3_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vk_1_2_features,
        .synchronization2 = true,
        .dynamicRendering = true,
    };
    VkPhysicalDeviceFeatures enabledVk10Features{
        .samplerAnisotropy = VK_TRUE
    };

    const std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo device_ci{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &vulkan_1_3_features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_ci,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = &enabledVk10Features
    };

    if( vkCreateDevice(this->physical_device, &device_ci, nullptr, &this->logical_device) != VK_SUCCESS) 
        throw std::runtime_error("Failed to create logical device");
    if constexpr (debug)
        std::cout << "Logical Device created" << std::endl;
}

bool Device::is_device_suitable(VkPhysicalDevice _device){
    VkPhysicalDeviceProperties2 properties{ .sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(_device, &properties);

    bool suitability = false;

    if( properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
        suitability = true;

    if( properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
        suitability = true;

    if( properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU )
        suitability = true;


    //if( properties.properties.limits )


    return suitability;
}

float Device::rate_device(VkPhysicalDevice _device){
    VkPhysicalDeviceProperties2 properties{ .sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(_device, &properties);
    VkPhysicalDeviceFeatures2 features{ .sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    vkGetPhysicalDeviceFeatures2(_device, &features);

    properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    float score = 0;

    if( properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
        score += 1000;

    if( properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
        score += 700;

    if( properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU )
        score += 250;

    return score;
}


} // namespace velora
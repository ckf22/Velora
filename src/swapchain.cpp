#include "swapchain.hpp"

#include <iostream>
#include <stdexcept>

namespace velora{


SwapChain::SwapChain(VkSurfaceKHR& _surface, Device& device, unsigned int _width = 800, unsigned int _height = 600)
 : surface{_surface}, device{device.get_device()} {
    this->extent = VkExtent2D{.width = _width, .height = _height};
    this->initialise_swapchain(device);
}

SwapChain::~SwapChain(){
    vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);

    //vkDestroyImage(this->device, this->images[0], nullptr);
    //vkFreeMemory(this->device, this->image_ram[0], nullptr);

    for(auto& it : this->image_views)
        vkDestroyImageView(this->device, it, nullptr);
}

void SwapChain::recreate_swapchain(unsigned int _width, unsigned int _height){
    this->extent = VkExtent2D{.width = _width, .height = _height};
}

void SwapChain::initialise_swapchain(Device& device) {
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_physical_device(), this->surface, &capabilities);

    this->swapchain_ci = VkSwapchainCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = this->surface,
        .minImageCount = capabilities.minImageCount,
        .imageFormat = this->choose_format(this->surface, device, capabilities),
        .imageExtent = {.width=this->extent.width, .height=this->extent.height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = this->choose_present_mode(this->surface, device, capabilities)
    };

    if( vkCreateSwapchainKHR(device.get_device(), &this->swapchain_ci, nullptr, &this->swapchain) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Swapchain");

    if constexpr (debug)
        std::cout << "Swapchain created" << std::endl;

    u_int32_t count{};
    vkGetSwapchainImagesKHR(this->device, this->swapchain, &count, nullptr);

    if( count == 0 )
        throw std::runtime_error("No Swapchain images found");

    this->images.resize(count);
    vkGetSwapchainImagesKHR(this->device, this->swapchain, &count, this->images.data());

    for(int i = 0; i < count; ++i){
        
    }
}

VkFormat SwapChain::choose_format(VkSurfaceKHR& _surface, Device& device, VkSurfaceCapabilitiesKHR capabilities){
    u_int32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_physical_device(), _surface, &count, nullptr);

    if(count == 0)
        throw std::runtime_error("No Surface Formats");

    std::vector<VkSurfaceFormatKHR> availible(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_physical_device(), _surface, &count, availible.data());

    std::optional<VkFormat> format;
    for(auto& it : availible){
        if(it.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && it.format == VK_FORMAT_B8G8R8A8_UNORM)
            format = it.format;
    }

    if(!format.has_value())
        throw std::runtime_error("No Supported Surface Formats");

    this->image_format = *format;
    return *format;
}

VkFormat SwapChain::choose_depth_format(Device& _device){

    std::optional<VkFormat> format;
    VkFormatProperties2 formatProperties{};
    for(auto& it : this->preferred_depth_formats){
        formatProperties = VkFormatProperties2{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        vkGetPhysicalDeviceFormatProperties2(_device.get_physical_device(), it, &formatProperties);
        if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            format = it;
            break;
        }
    }

    if( !format.has_value() )
        throw std::runtime_error("No ");

    this->depth_format = *format;
    return *format;
}

VkPresentModeKHR SwapChain::choose_present_mode(VkSurfaceKHR& _surface, Device& device, VkSurfaceCapabilitiesKHR capabilities){
    u_int32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_physical_device(), _surface, &count, nullptr);

    if( count == 0 )
        throw std::runtime_error("No Present Modes are Supported");

    std::vector<VkPresentModeKHR> availible(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_physical_device(), _surface, &count, availible.data());

    std::optional<VkPresentModeKHR> present_mode;
    for(auto& it : this->preffered_present_modes){
        for(auto& it2 : availible){
            if(it == it2)
                present_mode = it;
        }
        if(present_mode.has_value())
            break;
    }

    if( !present_mode.has_value() )
        throw std::runtime_error("No selected Present Mode is supported");

    return *present_mode;
}


}
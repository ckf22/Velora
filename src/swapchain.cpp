#include "swapchain.hpp"

#include <iostream>
#include <string>
#include <stdexcept>

namespace velora{

SwapChain::SwapChain(VkSurfaceKHR& _surface, Device& device, unsigned int _width = 800, unsigned int _height = 600)
 : surface{_surface}, device{device.get_device()} {
    this->extent = VkExtent2D{.width = _width, .height = _height};
    this->initialise_swapchain(device);
}

SwapChain::~SwapChain(){
    vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);

    for(auto& it : this->fences)
        vkDestroyFence(this->device, it, nullptr);

    for(auto& it : this->image_views)
        vkDestroyImageView(this->device, it, nullptr);

    for(auto& it : this->depth_image_views)
        vkDestroyImageView(this->device, it, nullptr);

    for(auto& it : this->depth_images)
        vkDestroyImage(this->device, it, nullptr);

    for(auto& it : this->depth_image_ram)
        vkFreeMemory(this->device, it, nullptr);
}

void SwapChain::aquire_next_image(VkSemaphore& image_ready_semaphore){
    u_int32_t i{};
    auto result = vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX, image_ready_semaphore, VK_NULL_HANDLE, &i);
    if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
        throw std::runtime_error("Failed to Aquire Next Image");

    this->current_index = static_cast<size_t>(i);
}

void SwapChain::wait_for_active_image_fence(){
    if( vkWaitForFences(this->device, 1, &this->fences[this->current_index], true, UINT64_MAX) != VK_SUCCESS )
        throw std::runtime_error(std::string("Failed to wait on Fence; index ")+std::to_string(this->current_index));
    if( vkResetFences(this->device, 1, &this->fences[this->current_index]) != VK_SUCCESS )
        throw std::runtime_error(std::string("Failed to reset Fence; index ")+std::to_string(this->current_index));
}

void SwapChain::recreate_swapchain(unsigned int _width, unsigned int _height){
    throw std::runtime_error("This function is not implemented yet");
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

    u_int32_t count{};
    vkGetSwapchainImagesKHR(this->device, this->swapchain, &count, nullptr);

    if( count == 0 )
        throw std::runtime_error("No Swapchain images found");

    this->images.resize(count);
    vkGetSwapchainImagesKHR(this->device, this->swapchain, &count, this->images.data());

    this->image_views.resize(count);
    for(int i = 0; i < count; ++i){
        VkImageViewCreateInfo image_view_ci{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = this->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = this->image_format,
            .subresourceRange{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 },
        };
        if( vkCreateImageView(this->device, &image_view_ci, VK_NULL_HANDLE, &this->image_views[i]) != VK_SUCCESS)
            throw std::runtime_error(std::string("Failed to create Image View at index ")+std::to_string(i));
    }


    VkImageCreateInfo depth_image_ci{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = this->choose_depth_format(device),
        .extent{.width = this->extent.width, .height = this->extent.height, .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    this->depth_images.resize(count);
    for(int i = 0; i < count; ++i){
        if(vkCreateImage(device.get_device(), &depth_image_ci, nullptr, &this->depth_images[i]) != VK_SUCCESS)
            throw std::runtime_error(std::string("Failed to create Depth Image at index ")+std::to_string(i));
    }

    if constexpr (debug)
        std::cout << "Depth Images Created" << std::endl;

    this->depth_image_views.resize(count);
    this->depth_image_ram.resize(count);
    for(int i = 0; i < count; ++i){
        VkImageViewCreateInfo depth_image_view_ci{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = this->depth_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = this->depth_format,
            .subresourceRange{ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 },
        };
        VkMemoryRequirements ram_requirements{};
        vkGetImageMemoryRequirements(device.get_device(), this->depth_images[i], &ram_requirements);
        VkMemoryAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = ram_requirements.size,
            .memoryTypeIndex = 0
        };
        vkAllocateMemory(device.get_device(), &alloc_info, VK_NULL_HANDLE, &this->depth_image_ram[i]);
        vkBindImageMemory(device.get_device(), this->depth_images[i], this->depth_image_ram[i], 0);
        if(vkCreateImageView(device.get_device(), &depth_image_view_ci, nullptr, &this->depth_image_views[i]) != VK_SUCCESS)
            throw std::runtime_error(std::string("Failed to create Depth Image View at index ")+std::to_string(i));
    }

    if constexpr (debug)
        std::cout << "Depth Image Views created" << std::endl;


    VkFenceCreateInfo fence_ci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    this->fences.resize(count);
    for(int i = 0; i < count; ++i){
        if(vkCreateFence(device.get_device(), &fence_ci, VK_NULL_HANDLE, &this->fences[i]) != VK_SUCCESS)
            throw std::runtime_error(std::string("Failed to create Fence at index ")+std::to_string(i));
        fence_ci.flags = 0; // only the first fence needs to be signalled due to the render loop
    }

    if constexpr (debug)
        std::cout << "Fences created" << std::endl;


    if constexpr (debug)
        std::cout << "Swapchain created" << std::endl;
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
        throw std::runtime_error("No suitable Depth Format found");

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

    if constexpr (debug)
        std::cout << "Present Mode: " << *present_mode << std::endl;

    return *present_mode;
}


}
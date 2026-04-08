#include "instance.hpp"

#include <stdexcept>
#include <iostream>

namespace velora{

Instance::Instance(){
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Velora",
        .apiVersion = VK_API_VERSION_1_3
    };

    this->create_instance(appInfo);

}
Instance::~Instance(){
    vkDestroyInstance(this->instance, nullptr);
}

void Instance::create_instance(VkApplicationInfo& app_info){
    auto extensions = this->get_required_extensions();
    VkInstanceCreateInfo instance_ci{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = static_cast<u_int32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };
    if( vkCreateInstance(&instance_ci, nullptr, &this->instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Instance");
}

std::vector<const char*> Instance::get_required_extensions(){
    uint32_t count{0};
    auto buffer = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char*> extensions{count};

    if constexpr( Instance::debug )
        std::cout << "Required Extensions" << std::endl;

    int i = 0;
    for(; i < count; ++i)
        extensions[i] = buffer[i];
        if constexpr( Instance::debug )
            std::cout << "    " << buffer[i] << std::endl;

    return extensions;
}

};
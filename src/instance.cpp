#include "instance.hpp"

#include <stdexcept>
#include <string>
#include <cstring>
#include <iostream>

namespace velora{

Instance::Instance(){
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Velora",
        .apiVersion = VK_API_VERSION_1_3
    };

    this->create_instance(appInfo);

    if constexpr (debug)
        std::cout << "Instance created" << std::endl;
}
Instance::~Instance(){
    vkDestroyInstance(this->instance, nullptr);
}

void Instance::create_instance(VkApplicationInfo& app_info){
    this->initialise_required_extensions();
    VkInstanceCreateInfo instance_ci{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = static_cast<u_int32_t>(this->required_extensions.size()),
        .ppEnabledExtensionNames = this->required_extensions.data()
    };

    if constexpr (debug) {
        instance_ci.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        instance_ci.ppEnabledLayerNames = validation_layers.data();

        std::cout << "Validation Layers enabled" << std::endl;
    } else {
        instance_ci.enabledLayerCount = 0;
    }

    this->check_glfw_extension_support(this->required_extensions);
    if( vkCreateInstance(&instance_ci, nullptr, &this->instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Instance");

}

void Instance::initialise_required_extensions(){
    uint32_t count = 0;
    const char ** data = glfwGetRequiredInstanceExtensions(&count);
    this->required_extensions = std::vector<const char*>(data, count + data);

    if constexpr (debug){
        std::cout << "Required Extensions" << std::endl;
        for(auto& it : this->required_extensions)
            std::cout << "\t" << it << std::endl;
    }

}

void Instance::check_glfw_extension_support(std::vector<const char*> required){
    uint32_t avalible_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &avalible_count, nullptr);
    std::vector<VkExtensionProperties> avalible(avalible_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &avalible_count, avalible.data());

    if constexpr(debug)
        std::cout << "Avalible Extensions" << std::endl;

    if constexpr(debug)
        for(auto& it : avalible){
            std::cout << '\t' << it.extensionName << "\t\t" << it.specVersion << std::endl;
    }

    for(auto& it : required){
        for(int i = 0; i <= avalible.size(); ++i){
            if(i == avalible.size())
                throw std::runtime_error( std::string("GLFW requires the following extension, which is not supported: ")+it );
            if(std::string(it) == std::string(avalible[i].extensionName))
                break;
        }
    }
}

bool Instance::check_validation_layer_support(){
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    std::vector<VkLayerProperties> available_layers(count);
    vkEnumerateInstanceLayerProperties(&count, available_layers.data());

    bool found = false;
    for (auto it : validation_layers) {

        found = false;
        for (auto& it2 : available_layers)
            if (strcmp(it, it2.layerName) == 0)
                found = true;
                break;

        if (!found)
            return false;
    }

    return true;
}

};

#pragma once

#include "device.hpp"
#include "descriptors.hpp"

namespace velora{

class TextureManager{
  public:
    TextureManager(Device& _device, Descriptors& _descriptors, VkCommandPool& cmd_pool, std::string filename);
    ~TextureManager();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    VkImage& get_image() { return texture; }
    VkImageView& get_image_view() { return image_view; }
    VkSampler& get_sampler() { return sampler; }
    void allocate_descriptors();
  private:
    void load_texture(VkCommandPool& cmd_pool, std::string filename);
    void register_descriptors();

    Device& device;
    Descriptors& descriptors;

    VkImage texture;
    VkDeviceMemory texture_ram;
    VkImageView image_view;
    VkSampler sampler;
};

}
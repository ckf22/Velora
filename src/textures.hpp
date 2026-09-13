#pragma once

#include "device.hpp"
#include "descriptors.hpp"

namespace velora{

class TextureManager{
  public:
    TextureManager(std::shared_ptr<Device> _device, Descriptors& descriptors, VkCommandPool& cmd_pool, std::string filename);
    ~TextureManager();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    VkImage& get_image() { return texture; }
    VkImageView& get_image_view() { return image_view; }
    VkSampler& get_sampler() { return sampler; }
    void allocate_descriptors(Descriptors& descriptors);
  private:
    void load_texture(VkCommandPool& cmd_pool, std::string filename);
    void register_descriptors(Descriptors& descriptors);

    std::shared_ptr<Device> device;

    VkImage texture;
    VkDeviceMemory texture_ram;
    VkImageView image_view;
    VkSampler sampler;
};

}
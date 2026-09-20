#pragma once

#include "device.hpp"

#include <memory>

namespace velora{

class Texture{
  public:
    Texture(std::shared_ptr<Device> device, const VkCommandPool& cmd_pool, std::string filename);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    const VkImage& get_image() const noexcept { return image; }
    const VkImageView& get_image_view() const noexcept { return image_view; }
    const VkDeviceMemory& get_ram() const noexcept { return ram; }
    const VkSampler& get_sampler() const noexcept { return sampler; }
  private:
    void load_texture(const VkCommandPool& cmd_pool, std::string filename);

    std::shared_ptr<Device> device;

    VkImage image;
    VkImageView image_view;
    VkDeviceMemory ram;

    VkSampler sampler;
};

}
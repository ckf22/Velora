#pragma once

#include "device.hpp"

#include <memory>

namespace velora{

class Texture{
  public:
    Texture(std::shared_ptr<Device> _device, const VkCommandPool& cmd_pool, std::string _filename);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    const VkImage& get_image() const noexcept { return image; }
    const VkImageView& get_image_view() const noexcept { return image_view; }
    const VkDeviceMemory& get_ram() const noexcept { return ram; }
  private:
    void load_texture(const VkCommandPool& cmd_pool, std::string filename);
  
    const std::string filename;
    const u_int64_t id;

    std::shared_ptr<Device> device;

    VkImage image;
    VkImageView image_view;
    VkDeviceMemory ram;
};

}
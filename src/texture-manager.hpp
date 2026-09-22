#pragma once

#include "device.hpp"
#include "texture.hpp"

#include <vector>

namespace velora{

class TextureManager{
  public:
    TextureManager(std::shared_ptr<Device> _device, const u_int32_t _frame_count, const VkCommandPool& cmd_pool, std::string directory);
    ~TextureManager();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    const VkDescriptorSetLayout& get_layout() const noexcept { return layout; }
    const std::size_t get_texture_count() const noexcept { return textures.size(); }

    void bind(const VkCommandBuffer& cmd_buffer, const VkPipelineLayout& layout, u_int32_t index);
  private:
    void load_directory(std::string directory, const VkCommandPool& pool);
    void generate_descriptor_ressources();
    void generare_sampler();

    std::shared_ptr<Device> device;
    const u_int32_t frame_count;

    VkSampler linear_sampler;

    // Because these descriptors require special setup
    VkDescriptorPool pool;
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSet> sets;

    std::vector<std::unique_ptr<Texture>> textures;

};

}
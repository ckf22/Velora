#pragma once

#include "device.hpp"
#include "buffer.hpp"

#include <vector>
#include <memory>

namespace velora{

class Descriptors{
  #ifdef DEBUG
  static constexpr bool debug = true;
  #else
  static constexpr bool debug = false;
  #endif
  public:
    // it is assumed that one descriptorSet is created per FrameInFlight and that they have the same contents
    Descriptors(Device& _device, const u_int32_t _descriptor_set_count);
    ~Descriptors();

    Descriptors(const Descriptors&) = delete;
    Descriptors& operator=(const Descriptors&) = delete;

    const u_int32_t get_set_count() const { return this->descriptor_set_count; }
    const VkDescriptorSetLayout& get_layout() const { return layout; }
    const VkDescriptorSet& get_set(u_int32_t index) const { return this->sets[index]; };

    void add_binding(VkDescriptorSetLayoutBinding binding);

    VkDescriptorSetLayout& generate_layout();
    void generate_sets();
    // the .dstSet required in 'VkWriteDescriptorSet' will be populated in the function according to the index
    void allocate_descriptor(VkWriteDescriptorSet write, u_int32_t index);
  private:
    bool ressources_created = false;
    const u_int32_t descriptor_set_count;
    Device& device;

    // For allocation
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkWriteDescriptorSet> allocate_info;
    std::vector<VkDescriptorPoolSize> pool_size;

    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSet> sets;

    VkDescriptorPool pool;
};

}
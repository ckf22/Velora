#pragma once

#include "device.hpp"
#include "buffer.hpp"

#include <vector>
#include <memory>

namespace velora{

class DescriptorManager;

class DescriptorPool{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
    friend class DescriptorManager;
  public:
    DescriptorPool(Device& _device, u_int32_t _max_sets, std::vector<VkDescriptorPoolSize>& pool_sizes);
    ~DescriptorPool();

    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;

    const u_int32_t get_max_sets() const { return max_sets; }

    void create_temp();
    void add_b(MyBuffer& buffer);
  private:
    Device& device;
    const u_int32_t max_sets = 0;

    VkDescriptorPool descriptor_pool;
};


class DescriptorManager{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
    struct DescriptorSetCreateData{
      int layout_id;
      u_int32_t count;
    };
  public:
    struct DescriptorSetData{
      VkDescriptorSet set;
      VkDescriptorSetAllocateInfo alloc_info;
    };
    struct DescriptorSetLayoutData{
      VkDescriptorSetLayout layout;
      VkDescriptorSetLayoutCreateInfo ci;
    };

    DescriptorManager(Device& _device);
    ~DescriptorManager();

    DescriptorManager& operator=(const DescriptorManager&) = delete;
    DescriptorManager(const DescriptorManager&) = delete;

    DescriptorPool& get_pool(){ return *this->pool; }
    VkDescriptorSet& get_set(int id){ return this->sets[id].set; }
    VkDescriptorSetLayout& get_layout(int id) { return this->layouts[id].layout; }
    std::vector<VkDescriptorSetLayout> get_layout_vector();

    void allocate_buffer_descriptor(int target_set, MyBuffer& buffer, u_int32_t shader_binding, u_int32_t buffer_offset = 0);

    // used for constructing
    void create_ressources();
    int add_layout(std::vector<VkDescriptorSetLayoutBinding> binding_infos, VkDescriptorSetLayoutCreateFlags flags = 0);
    int add_descriptor_set(int layout_id, u_int32_t set_count); // returns index of descriptor set
    void add_descriptor(VkDescriptorType descriptor_type, u_int32_t descriptor_count);
  private:
    Device& device;
    bool ressources_created_flag = false;

    std::unique_ptr<DescriptorPool> pool;
    std::vector<DescriptorSetData> sets;
    std::vector<DescriptorSetLayoutData> layouts;

    // used for constructing
    std::vector<VkDescriptorPoolSize> pool_sizes;
    std::vector<DescriptorSetCreateData> set_create_data;
};

}
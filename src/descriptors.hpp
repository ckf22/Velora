#pragma once

#include "device.hpp"
#include "buffer.hpp"

#include <vector>
#include <memory>

namespace velora{

class DescriptorSet;
class DescriptorManager;

class DescriptorPool{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
    friend class DescriptorSet;
    friend class DescriptorManager;
  public:
    DescriptorPool(Device& _device, u_int32_t _max_sets);
    ~DescriptorPool();

    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;

    const u_int32_t get_max_sets() const { return max_sets; }
    // temp
    const VkDescriptorSetLayout& get_layout() const { return layout; }
    VkDescriptorSet& get_set() { return set; }

    void create_temp();
    void add_b(MyBuffer& buffer);
  private:
    Device& device;
    const u_int32_t max_sets = 0;

    VkDescriptorPool descriptor_pool;

    // temp
    VkDescriptorSetLayout layout;
    VkDescriptorSet set;
};

class DescriptorSet{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
    friend class DescriptorPool;
    friend class DescriptorManager;
  public:
    DescriptorSet(Device& _device, DescriptorPool& _pool);
    ~DescriptorSet();

    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet& operator=(const DescriptorSet&) = delete;

    const VkDescriptorSetLayout& get_layout() const { return this->layout; }
    const VkDescriptorSet& get_set() const { return this->set; }
  private:
    Device& device;
    DescriptorPool& pool;

    VkDescriptorSetLayout layout;
    VkDescriptorSet set;
};

class DescriptorManager{
  public:
    DescriptorManager(Device& _device);
    ~DescriptorManager();

    void add_descriptor_set();

    void create_ressources();
  private:
    Device& device;

    std::unique_ptr<DescriptorPool> pool;
    std::vector<std::unique_ptr<DescriptorSet>> sets;
};

}
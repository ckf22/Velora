#include "device.hpp"
#include "object.hpp"

#include <vector>
#include <chrono>

namespace velora{

class ObjectManager{
    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif
  public:
    ObjectManager(Device& _device, const u_int32_t _frame_count,  std::vector<Vertex> object);
    ~ObjectManager();

    ObjectManager& operator=(ObjectManager&) = delete;
    ObjectManager(ObjectManager&) = delete;

    u_int32_t get_vertex_count(){ return vertices.size(); }

    void update_shader_data(std::chrono::microseconds dt, u_int32_t target_index);
    void bind_cmd_buffer(VkCommandBuffer& cmd_buffer, u_int32_t index);
  private:
    void create_buffer();

    const u_int32_t frame_count;
    Device& device;

    std::vector<VkBuffer> vertex_buffers;
    std::vector<VkDeviceMemory> vertex_buffer_memory;

    std::vector<Vertex> vertices;
    u_int32_t vertex_count = 0;
};

}
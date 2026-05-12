#pragma once

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>

namespace velora{

struct Vertex{
    glm::vec2 position;
    glm::vec3 color;
    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
    static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();

    static std::vector<Vertex> get_default_triangle(){
      return {
        {.position={0, -0.5},   .color={0.2f,0.2f,0.8f}},
        {.position={-0.5, 0.5}, .color={0.2f,0.8f,0.2f}},
        {.position={0.5, 0.5},  .color={0.8f,0.2f,0.2f}}
      };
      /*
      return {
        {{0,-0.5}},
        {{-0.3535533906, 0.3535533906}},
        {{0.3535533906, 0.3535533906}},
        {{1,1}},
        {{0.8,1}},
        {{1,0.8}}
      };*/
    }
};

class Object{
  public:
    Object(std::vector<Vertex> data, const u_int32_t _max_indices);

    glm::mat4 get_object_transform();
    u_int32_t write_data_to_buffer(void * destination);

    const u_int32_t get_max_vertex_count() const { return this->max_indices; }
    u_int32_t get_vertex_count() { return this->vertices.size(); }
    std::vector<Vertex>& get_vertices() { return this->vertices; }
  private:
    std::vector<Vertex> vertices;
    //std::vector<u_int32_t> indices;

    const u_int32_t max_indices;

    glm::vec3 translation{0};
    glm::vec3 rotation{0};  // rotation around x achsis, then y, z
    glm::vec3 rotation_anchor{0};
};

}
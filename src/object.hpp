#pragma once

#include "transform-component.hpp"

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace velora{

struct Vertex{
    glm::vec3 position{0.f};
    glm::vec3 color{.1f};

    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
    static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
};

class Object{
  public:
    static Object get_default_cube(glm::vec3 offset = {0,0,0});

    Object(std::vector<Vertex> _vertices, std::vector<u_int32_t> _indices, const u_int32_t _max_vertices, const u_int32_t _max_indices);

    //glm::mat4 get_transform();
    u_int32_t write_vertex_data(void * destination);
    u_int32_t write_index_data(void * destination);

    const u_int32_t get_max_vertex_count() const { return this->max_vertices; }
    const u_int32_t get_max_index_count() const { return this->max_indices; }
    u_int32_t get_index_count() { return this->indices.size(); }
    u_int32_t get_vertex_count() { return this->vertices.size(); }
    u_int32_t get_required_ram() { return this->vertices.size() * sizeof(Vertex); }
    std::vector<Vertex>& get_vertices() { return this->vertices; }
  private:
    std::vector<Vertex> vertices;
    std::vector<u_int32_t> indices;

    const u_int32_t max_vertices;
    const u_int32_t max_indices;

    //TransformComponent transform;
};

}
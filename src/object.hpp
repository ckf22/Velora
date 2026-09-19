#pragma once

#include "vertex.hpp"
#include "transform-component.hpp"

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <optional>

namespace velora{

class Object{
  public:
    // temp
    static Object perlin_noise(glm::vec3 scale, float octave_length, u_int32_t extent, u_int32_t density);

    static Object get_default_cube(glm::vec3 offset = {0,0,0});
    static Object load_file(std::string filename);

    Object(std::vector<Vertex>& _vertices, std::vector<u_int32_t>& _indices, const u_int32_t _max_vertices, const u_int32_t _max_indices);
    Object(std::vector<Vertex>& _vertices, std::vector<u_int32_t>& _indices, std::vector<TransformComponent>& _transforms,
         const u_int32_t _max_vertices, const u_int32_t _max_indices);

    u_int32_t write_vertex_data(void * destination, bool force = false);
    u_int32_t write_index_data(void * destination, bool force = false);
    u_int32_t write_transform_data(void * destination, bool force = false);

    void update(std::chrono::microseconds dt);

    const u_int32_t get_max_vertex_count() const { return this->max_vertices; }
    const u_int32_t get_max_index_count() const { return this->max_indices; }
    u_int32_t get_index_count() { return this->indices.size(); }
    u_int32_t get_vertex_count() { return this->vertices.size(); }
    u_int32_t get_required_ram() { return this->vertices.size() * sizeof(Vertex); }
    u_int32_t get_transform_count() { return this->transforms.size(); }
    std::vector<Vertex>& get_vertices() { return this->vertices; }
    std::vector<u_int32_t>& get_indices() { return this->indices; }
    std::vector<TransformComponent>& get_transforms() { return this->transforms; }
  private:
    std::vector<Vertex> vertices;
    std::vector<u_int32_t> indices;

    const u_int32_t max_vertices;
    const u_int32_t max_indices;

    bool updated = false;
    std::optional<std::function<void(void*)>> update_funtion;
    std::optional<void*> update_function_parameter;

    std::vector<TransformComponent> transforms;
};

}
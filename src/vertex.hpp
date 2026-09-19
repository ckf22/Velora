#pragma once

#define GLM_FORCE_RADIANS
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

struct VkVertexInputAttributeDescription;
struct VkVertexInputBindingDescription;

namespace velora{

struct Vertex{
    glm::vec3 position{0.f};
    glm::vec3 color{.5f};
    glm::vec3 normal{0,-1.f,0};
    glm::vec2 uv{.0f};

    bool operator==(const Vertex& other) const;
    bool operator==(Vertex&& other);

    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
    static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
};

}
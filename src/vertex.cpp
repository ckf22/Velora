#include "vertex.hpp"

#include <vulkan/vulkan.hpp>

namespace velora{

std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_descriptions(){
    VkVertexInputAttributeDescription a1{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, position)};
    VkVertexInputAttributeDescription a2{.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color)};
    VkVertexInputAttributeDescription a3{.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal)};
    VkVertexInputAttributeDescription a4{.location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv)};
    return {a1, a2, a3, a4};
}

std::vector<VkVertexInputBindingDescription> Vertex::get_binding_descriptions(){
    VkVertexInputBindingDescription a1 = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return {a1};
}

bool Vertex::operator==(const Vertex& other) const {
    return (this->position == other.position && this->color == other.color && this->normal == other.normal && this->uv == other.uv);
}

bool Vertex::operator==(Vertex&& other){
    return (this->position == other.position && this->color == other.color && this->normal == other.normal && this->uv == other.uv);
}

}
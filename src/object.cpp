#include "object.hpp"

namespace velora{

Object::Object(){
}

std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_descriptions(){
    VkVertexInputAttributeDescription a1{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, position)};
    VkVertexInputAttributeDescription a2{.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color)};
    return {a1,a2};
}

std::vector<VkVertexInputBindingDescription> Vertex::get_binding_descriptions(){
    VkVertexInputBindingDescription a1 = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return {a1};
}

} // namespace velora
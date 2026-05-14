#include "object.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <string.h>

namespace velora{

Object::Object(std::vector<Vertex> data, const u_int32_t _max_indices) : max_indices{_max_indices} {
}

glm::mat4 Object::get_transform(){
    return glm::mat4{1.f};
}
u_int32_t Object::write_data(void * destination){
    u_int32_t bytes = sizeof(Vertex)*this->vertices.size();
    memcpy(destination, this->vertices.data(), bytes);
    return bytes;
}


std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_descriptions(){
    //VkVertexInputAttributeDescription a1_0{.location = 0, .binding = 0, .format = VK_FORMAT_R32G3, .offset = offsetof(Vertex, object_transform)};
    VkVertexInputAttributeDescription a2{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, position)};
    VkVertexInputAttributeDescription a3{.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color)};
    return {a2, a3};
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
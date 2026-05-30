#include "object.hpp"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <string.h>

namespace velora{

Object::Object(std::vector<Vertex> _vertices, std::vector<u_int32_t> _indices, const u_int32_t _max_vertices, const u_int32_t _max_indices)
 : max_indices{_max_indices}, max_vertices{_max_vertices}, vertices{_vertices}, indices{_indices} {
    this->transforms.push_back( TransformComponent() );
 }

Object::Object(std::vector<Vertex> _vertices, std::vector<u_int32_t> _indices, std::vector<TransformComponent> _transforms, const u_int32_t _max_vertices, const u_int32_t _max_indices)
 : max_indices{_max_indices}, max_vertices{_max_vertices}, vertices{_vertices}, indices{_indices}, transforms{_transforms} {}

u_int32_t Object::write_vertex_data(void * destination){
    u_int32_t bytes = sizeof(Vertex)*this->vertices.size();
    memcpy(destination, this->vertices.data(), bytes);
    return bytes;
}

u_int32_t Object::write_index_data(void * destination){
    u_int32_t bytes = sizeof(u_int32_t)*this->indices.size();
    mempcpy(destination, this->indices.data(), bytes);
    return bytes;
}

u_int32_t Object::write_transform_data(void * destination){
    glm::mat4 buffer;
    for(auto& it : this->transforms){
        buffer = it.get_transform();
        memcpy(destination, &buffer, sizeof(glm::mat4));
        destination += sizeof(glm::mat4);
    }
    return sizeof(glm::mat4) * this->transforms.size();
}


Object Object::get_default_cube(glm::vec3 offset){
    // left face (white)
    std::vector<Vertex> vertices{
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    };

    for(auto& it : vertices)
        it.position += offset;

    std::vector<u_int32_t> indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                          12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

    return Object(vertices, indices, vertices.size(), indices.size());
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
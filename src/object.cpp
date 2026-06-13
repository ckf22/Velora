#include "object.hpp"

#include "libs/hash.hpp"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "libs/tiny_obj_loader.h"

#include <memory>
#include <string.h>
#include <iostream>
#include <unordered_map>
#include <chrono>


namespace velora{

Object::Object(std::vector<Vertex>& _vertices, std::vector<u_int32_t>& _indices, const u_int32_t _max_vertices, const u_int32_t _max_indices)
 : max_indices{_max_indices}, max_vertices{_max_vertices}, vertices{_vertices}, indices{_indices} {
    this->transforms.push_back( TransformComponent() );
 }

Object::Object(std::vector<Vertex>& _vertices, std::vector<u_int32_t>& _indices, std::vector<TransformComponent>& _transforms, const u_int32_t _max_vertices, const u_int32_t _max_indices)
 : max_indices{_max_indices}, max_vertices{_max_vertices}, vertices{_vertices}, indices{_indices}, transforms{_transforms} {}

u_int32_t Object::write_vertex_data(void * destination, bool force){
    u_int32_t bytes = sizeof(Vertex)*this->vertices.size();
    if( this->updated == true || force == true ){
        memcpy(destination, this->vertices.data(), bytes);
        this->updated = false;
    }
    return bytes;
}

u_int32_t Object::write_index_data(void * destination, bool force){
    u_int32_t bytes = sizeof(u_int32_t)*this->indices.size();
    if( this->updated == true || force == true ){
        mempcpy(destination, this->indices.data(), bytes);
        this->updated = false;
    }
    return bytes;
}

u_int32_t Object::write_transform_data(void * destination, bool force){
    if( this->updated == true || force == true ){
        glm::mat4 buffer;
        for(auto& it : this->transforms){
            buffer = it.get_transform();
            memcpy(destination, &buffer, sizeof(glm::mat4));
            destination += sizeof(glm::mat4);
        }
        this->updated = false;
    }
    return sizeof(glm::mat4) * this->transforms.size();
}

void Object::update(std::chrono::microseconds dt){
    if( this->update_funtion.has_value() ){
        void * parameter = nullptr;
        if(this->update_function_parameter.has_value())
            parameter = *this->update_function_parameter;

        //this->update_funtion->operator(parameter);
        this->updated = true;
    }
}

Object Object::load_file(std::string filename){
    auto t0 = std::chrono::high_resolution_clock::now();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    if( !tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filename.c_str()) )
        throw std::runtime_error(warning + error);

    constexpr int increment_step = 1000;
    
    std::vector<Vertex> vertices(increment_step);
    std::vector<u_int32_t> indices(increment_step);

    // Stores Vertices and the index the have
    std::unordered_map<Vertex, u_int32_t, hash::hash_struct<Vertex>> vertex_register({});

    u_int32_t color_index, vertices_index = 0, indices_index = 0;
    for(auto& it : shapes){
        for(auto& it2 : it.mesh.indices){
            Vertex push;

            // position
            if( it2.vertex_index >= 0 )
                push.position = {
                    attrib.vertices[3*it2.vertex_index],
                    attrib.vertices[3*it2.vertex_index+1],
                    attrib.vertices[3*it2.vertex_index+2]
                };

            color_index = 3*it2.vertex_index+2; 
            if( color_index < attrib.colors.size() )
                push.color = {
                    attrib.colors[color_index-2],
                    attrib.colors[color_index-1],
                    attrib.colors[color_index]
                };

            if( it2.normal_index >= 0 )
                push.normal = {
                    attrib.normals[3*it2.normal_index],
                    attrib.normals[3*it2.normal_index+1],
                    attrib.normals[3*it2.normal_index+2]
                };

            if( it2.texcoord_index >= 0 )
                push.uv = {
                    attrib.texcoords[2*it2.texcoord_index],
                    attrib.texcoords[2*it2.texcoord_index+1]
                };

            // 'indices_index' has reached the end of the vector
            if(indices_index == indices.size())
                indices.resize(indices.size()+increment_step);

            auto it = vertex_register.find(push);

            // appending the index stored in the value of 'vertex_register'
            if(it != vertex_register.end()){
                indices[indices_index] = it->second;
                indices_index++;
            }

            // append the new vertex to the list, add it's index to 'indices' and add an entry to 'vertex_register'
            else{
                if(vertices_index == vertices.size())
                    vertices.resize(vertices.size()+increment_step);

                vertices[vertices_index] = push;
                vertices_index++;

                indices[indices_index] = vertices_index-1;
                indices_index++;

                vertex_register.insert({push, vertices_index-1});
            }
        }
    }

    vertices.resize(vertices_index);
    indices.resize(indices_index);

    // normalising normal vectors
    for(auto& it : vertices)
        it.normal = glm::normalize(it.normal);

    std::cout << "Loaded Model from file \'" << filename << "\'(" 
              << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0).count()
              << "ms):\n" << vertices.size() << " Unique Vertices; " << indices.size() / 3 << " Triangles;" << std::endl;

    return Object{vertices, indices, static_cast<u_int32_t>(vertices.size()), static_cast<u_int32_t>(indices.size())};
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


} // namespace velora
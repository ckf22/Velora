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


// temp

#include <stdio.h>
#include <stdlib.h>


float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

Object Object::perlin_noise(glm::vec3 scale, float octave_length, u_int32_t extent, u_int32_t density) {
    auto t0 = std::chrono::high_resolution_clock::now();
    
    std::vector<glm::vec2> direction_vectors(extent * extent);
    srand(static_cast<unsigned int>(time(0)));

    for (auto& it : direction_vectors) {
        float radians = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / glm::two_pi<float>()));
        it.x = glm::cos(radians);
        it.y = glm::sin(radians);
    }

    u_int32_t total_points_per_axis = (extent - 1) * density + 1;

    std::vector<Vertex> vertices(total_points_per_axis*total_points_per_axis);

    // total points per axis are the edges of the imaginary quads, there total_points_per_axis-1 'quads' exist along one axis
    // it takes six indices to draw the two triangles in a quad
    std::vector<u_int32_t> indices((total_points_per_axis-1)*(total_points_per_axis-1)*6);

    for (u_int32_t g_x = 0; g_x < total_points_per_axis; ++g_x) {
        for (u_int32_t g_z = 0; g_z < total_points_per_axis; ++g_z) {
            
            float fx = static_cast<float>(g_x) / static_cast<float>(density);
            float fz = static_cast<float>(g_z) / static_cast<float>(density);

            u_int32_t x0 = static_cast<u_int32_t>(std::floor(fx));
            u_int32_t z0 = static_cast<u_int32_t>(std::floor(fz));
            
            u_int32_t x1 = (x0 >= extent - 1) ? x0 : x0 + 1;
            u_int32_t z1 = (z0 >= extent - 1) ? z0 : z0 + 1;

            float tx = fx - static_cast<float>(x0);
            float tz = fz - static_cast<float>(z0);

            float dot00 = glm::dot(direction_vectors[x0 * extent + z0], {tx, tz});
            float dot10 = glm::dot(direction_vectors[x1 * extent + z0], {tx - 1.0f, tz});
            float dot01 = glm::dot(direction_vectors[x0 * extent + z1], {tx, tz - 1.0f});
            float dot11 = glm::dot(direction_vectors[x1 * extent + z1], {tx - 1.0f, tz - 1.0f});

            float i1 = glm::mix(dot00, dot10, fade(tx));
            float i2 = glm::mix(dot01, dot11, fade(tx));


            vertices[(g_x*total_points_per_axis)+g_z].position = glm::vec3{ fx * octave_length, glm::mix(i1, i2, fade(tz)), fz * octave_length } * scale;
            vertices[(g_x*total_points_per_axis)+g_z].normal = {0,0,0};
        }
    }
    long index = 0;
    for (u_int32_t r = 0; r < total_points_per_axis - 1; ++r) {
        for (u_int32_t c = 0; c < total_points_per_axis - 1; ++c) {
            u_int32_t top_left     = r * total_points_per_axis + c;
            u_int32_t top_right    = top_left + 1;
            u_int32_t bottom_left  = (r + 1) * total_points_per_axis + c;
            u_int32_t bottom_right = bottom_left + 1;

            // counter clockwise
            indices[(index++)] = top_left;
            indices[(index++)] = bottom_left;
            indices[(index++)] = top_right;

            indices[(index++)] = top_right;
            indices[(index++)] = bottom_left;
            indices[(index++)] = bottom_right;
        }
    }
    if(index != indices.size()) // minus one because 'index' stops at the laast element
        throw std::runtime_error(std::string("Indexing error while generating perlin noise: index: ")+
            std::to_string(index)+std::string("; size: ")+std::to_string(indices.size())+';');

    for (size_t i = 0; i < indices.size(); i += 3) {
        u_int32_t idx0 = indices[i];
        u_int32_t idx1 = indices[i + 1];
        u_int32_t idx2 = indices[i + 2];

        glm::vec3 face_normal = glm::cross(vertices[idx1].position-vertices[idx0].position,
                                             vertices[idx2].position-vertices[idx0].position);

        vertices[idx0].normal += face_normal;
        vertices[idx1].normal += face_normal;
        vertices[idx2].normal += face_normal;
    }

    for (auto& it : vertices) {
        if (glm::length(it.normal) > 0.0001f)
            it.normal = glm::normalize(it.normal);
        else 
            it.normal = glm::vec3(0.0f, -1.0f, 0.0f); // Default straight up
    }

    std::cout << "Perlin noise generated; Octave length: " << octave_length << "; Tile count: " << extent*extent 
              << "; Vertex count: " << vertices.size() << ";("
              << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0).count()
              << "ms)" << std::endl;

    return Object(vertices, indices, vertices.size(), indices.size());
}

} // namespace velora
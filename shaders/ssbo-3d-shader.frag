#version 460

layout(location = 0) in vec3 color;
layout(location = 1) in vec3 world_pos;
layout(location = 2) in vec3 normal_out;

layout(location = 0) out vec4 result;

layout(set = 0, binding = 1, std140) uniform UBO {
    vec3 light_direction;
    float ambient;
    vec3 light_color;
} ubo;


void main(){
    result = vec4(color, 1);
}
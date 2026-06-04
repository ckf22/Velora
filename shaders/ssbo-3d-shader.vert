#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal_in;
layout(location = 3) in vec2 uv;


layout(push_constant) uniform Push{
    mat4 projection;
} push;

layout(set = 0, binding = 1, std140) uniform UBO {
    vec3 light_direction;
    float ambient;
    vec3 light_color;
} ubo;

layout(set = 0, binding = 2, std430) readonly buffer WorldSpaceTransforms{
    mat4 matrices[];
};


layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 world_pos;
layout(location = 2) out vec3 normal_out;


void main(){
    out_color = color;

    mat4 transform = matrices[ gl_InstanceIndex ];
    world_pos = (transform * vec4(position, 1.0)).xyz;

    gl_Position = push.projection * vec4(world_pos, 1.0);

    normal_out = normalize((transform * vec4(normal_in, 0)).xyz);

}

#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;


layout(push_constant) uniform Push{
    mat4 projection;
} push;

layout(set = 0, binding = 1, std430) readonly buffer WorldSpaceTransforms{
    mat4 matrices[];
};


layout(location = 0) out vec3 out_color;


void main(){
    out_color = color;
    mat4 transform = matrices[ gl_InstanceIndex ];
    gl_Position = push.projection * (transform * vec4(position, 1.0));
}

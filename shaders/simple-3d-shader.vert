#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(push_constant) uniform Push{
    mat4 projection;
    mat4 world_space;
} push;

layout(location = 0) out vec3 out_color;

void main(){
    out_color = color;

    gl_Position = push.projection * (push.world_space * vec4(position, 1.0));
}
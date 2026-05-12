#version 450

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 out_color;

layout(push_constant) uniform Push{
    mat4 projection;
    vec4 light_color;
    vec3 light_direction;
    vec3 ambient_light;
} push;

void main(){
    out_color = color;

    gl_Position = vec4(pos, 0, 1);
}
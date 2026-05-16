#version 450

layout(location = 0) in vec3 color;

layout(push_constant) uniform Push{
    mat4 projection;
    mat4 world_space;
} push;

layout(location = 0) out vec4 out_color;

void main(){
    out_color = vec4(color, 1);
}
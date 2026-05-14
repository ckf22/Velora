#version 450

layout(location = 0) in vec3 color;

layout(push_constant) uniform Push{
    mat4 projection;
    vec4 light_color;
    vec3 light_direction;
    vec3 ambient_light;
} push;

layout(location = 0) out vec4 out_color;

void main(){
    out_color = vec4(color+push.ambient_light, 1);
}
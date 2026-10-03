#version 460

layout(location = 0) in vec2 offset;
layout(location = 1) in vec3 in_color;

layout(push_constant) uniform Push {
    mat4 projection_view;
    vec3 camera_up;
    float cut_off_distance_squared;
    vec3 camera_right;
} push;


layout(location = 0) out vec4 out_color;

void main(){
    // Radius is accounted for in the vertex shader
    if( dot(offset, offset) >= push.cut_off_distance_squared ){
        discard;
    }
    out_color = vec4(in_color,1);
}
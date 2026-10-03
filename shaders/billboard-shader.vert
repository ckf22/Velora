/*
    This shader is to be called with six invocations and one instance to render one billboard
*/

#version 460

layout(location = 0) in vec3 world_pos;
layout(location = 1) in vec3 color;
layout(location = 2) in float radius;


layout(push_constant) uniform Push {
    mat4 projection_view;
    vec3 camera_up;
    float cut_off_distance_squared;
    vec3 camera_right;
} push;


layout(location = 0) out vec2 offset_out;
layout(location = 1) out vec3 out_color;

const vec2 offsets[6] = vec2[](
    vec2(1,1),  // top right
    vec2(-1,-1),  // bottom left
    vec2(-1,1), // top left

    vec2(1,-1), // bottom right
    vec2(1,1), // top right
    vec2(-1,-1) // bottom left
);


void main(){
    offset_out = offsets[gl_VertexIndex];
    out_color = color;

    vec3 result = world_pos
        + (push.camera_right * offset_out.x * radius)
        + (push.camera_up * offset_out.y * radius);

    gl_Position = push.projection_view * vec4(result,1.0);
}
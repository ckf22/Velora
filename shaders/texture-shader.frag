#version 460

layout(location = 0) in vec3 color;
layout(location = 1) in vec3 world_pos;
layout(location = 2) in vec3 normal_in;
layout(location = 3) in vec2 uv_in;

layout(location = 0) out vec4 result;

layout(set = 0, binding = 1, std140) uniform UBO {
    vec3 light_direction;
    float ambient;
    vec3 light_color;
    uint point_light_count;
} ubo;

struct PointLightInformation{
    vec3 position;
    float intensity;
    vec3 color;
    float range;
};

layout(set = 0, binding = 3, std430) readonly buffer PointLights{
    PointLightInformation point_lights[];
};

layout(binding = 4) uniform sampler2D textures;

void main(){
    vec3 normal = normalize(normal_in);

    vec3 texture_color = texture(textures, uv_in).xyz; 

    // directional light
    vec3 running_sum = (texture_color*ubo.light_color) * max(  dot(normal, ubo.light_direction),  ubo.ambient  );

    // point lights
    float dist;
    vec3 light_buffer;
    for(uint i = 0; i < ubo.point_light_count; i++){
        dist = max( distance(world_pos, point_lights[i].position), 0.01 ); // so no division by zero occurs
        if( dist < point_lights[i].range ){
            light_buffer = texture_color * point_lights[i].color;
            light_buffer = light_buffer * ( point_lights[i].intensity * max(0,dot(normal, normalize(point_lights[i].position - world_pos))) / (dist*dist) );
            running_sum += light_buffer;
        }
    }

    result = vec4(running_sum, 1);
}
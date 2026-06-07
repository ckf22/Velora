#version 460

layout(location = 0) in vec3 color;
layout(location = 1) in vec3 world_pos;
layout(location = 2) in vec3 normal_in;

layout(location = 0) out vec4 result;

layout(set = 0, binding = 1, std140) uniform UBO {
    vec3 light_direction;
    float ambient;
    vec3 light_color;
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

void main(){
    vec3 normal = normalize(normal_in);

    // directional light
    vec3 running_sum = (color*ubo.light_color) * max(  dot(normal, ubo.light_direction),  ubo.ambient  );

    // point lights
    float dist;
    for(uint i = 0; i < point_lights.length(); i++){
        dist = max( distance(world_pos, point_lights[i].position), 0.01 ); // so no division by zero occurs
        if( dist < point_lights[i].range ){
            running_sum += ((color * point_lights[i].color) * max(0,dot(normal, normalize(point_lights[i].position - world_pos))) / (dist*dist));
        }
    }

    result = vec4(running_sum, 1);
}
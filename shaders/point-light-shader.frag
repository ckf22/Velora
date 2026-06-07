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
    float intensity = max(  dot(normal, ubo.light_direction),  ubo.ambient  );
    vec3 running_sum = (color*ubo.light_color)*intensity;


    for(int i = 0; i < point_lights.length(); i += 1){
        if( distance(world_pos, point_lights[i].position) < point_lights[i].range ){
            running_sum += ((color * point_lights[i].color) * max(0,dot(normal, normalize(point_lights[i].position - world_pos))));
        }
    }

    result = vec4(running_sum, 1);
}
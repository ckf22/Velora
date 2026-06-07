#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace velora{

struct PointLight{
    glm::vec3 position{0.f};
    float intensity = 1;
    glm::vec3 color{.9f};
    float range = 1000;
};

}
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace velora{

struct Camera{
    glm::mat4 orthographic_projection(float near, float far, float left, float right, float bottom, float top);
    glm::mat4 orthographic_projection(glm::vec3 c1, glm::vec3 c2);
    glm::mat4 projection{1.f};
};

}
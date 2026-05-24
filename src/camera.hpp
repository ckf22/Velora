#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <optional>

namespace velora{

struct Camera{
    glm::mat4 orthographic_projection(float near, float far, float left, float right, float bottom, float top);
    glm::mat4 orthographic_projection(glm::vec3 c1, glm::vec3 c2);
    glm::mat4 perspective_projection(float near, float far, float theta, float aspect);

    glm::mat4 view_angles(glm::vec3 position, glm::vec3 rotation);
    //glm::mat4 view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up);
    //glm::mat4 view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up);

    glm::mat4 get_projection_view_matrix();

    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};

    glm::vec3 position{.0f};
    glm::vec3 rotation{.0f};
    //std::optional<glm::vec3> direction;
    //std::optional<glm::vec3> up; 
};

}
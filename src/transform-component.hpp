#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace velora{

class TransformComponent{
  public:
    TransformComponent(glm::vec3 _translation = {0,0,0}, glm::vec3 _scale = {1,1,1}, glm::vec3 _rotation = {0,0,0}, glm::vec3 _anchor = {0,0,0} );

    glm::vec3 translation{0.f};
    glm::vec3 anchor{0.f};
    glm::vec3 rotation{0.f};
    glm::vec3 scale{1.f};

    glm::mat4 get_transform(bool updated = false);
  private:
    void update_transform();

    glm::mat4 transform{1.f};
};

}
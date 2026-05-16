#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/projection.hpp>

#include <cassert>
#include <limits>

namespace velora{

glm::mat4 Camera::orthographic_projection(float near, float far, float left, float right, float bottom, float top){
    this->projection = {
        {2/(right-left), 0, 0, (-right-left)/(right-left)},
        {0, 2/(bottom-top), 0, (-bottom-top)/(bottom-top)},
        {0, 0, 2/(far-near),   (-near)/(far-near)},
        {0, 0 , 0 ,1}
    };
    return this->projection;
}

glm::mat4 Camera::orthographic_projection(glm::vec3 c1, glm::vec3 c2){
    return this->orthographic_projection(
        glm::min(c1.z, c2.z), glm::max(c1.z, c2.z),
        glm::min(c1.x, c2.x), glm::max(c1.x, c2.x),
        glm::min(c1.y, c2.y), glm::max(c1.y, c2.y)
    );
}

glm::mat4 Camera::perspective_projection(float near, float far, float theta, float aspect) {
  assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  const float tanHalfFovy = tan(glm::radians(theta) / 2.f);
  this->projection = glm::mat4{0.0f};
  this->projection[0][0] = 1.f / (aspect * tanHalfFovy);
  this->projection[1][1] = 1.f / (tanHalfFovy);
  this->projection[2][2] = far / (far - near);
  this->projection[2][3] = 1.f;
  this->projection[3][2] = -(far * near) / (far - near);
  return this->projection;
}

} // namespace velora
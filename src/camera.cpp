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

glm::mat4 Camera::view_angles(glm::vec3 position, glm::vec3 rotation){
  this->rotation = rotation;
  this->position = position;

  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
  const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
  const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
  this->view = glm::mat4{1.f};
  this->view[0][0] = u.x;
  this->view[1][0] = u.y;
  this->view[2][0] = u.z;
  this->view[0][1] = v.x;
  this->view[1][1] = v.y;
  this->view[2][1] = v.z;
  this->view[0][2] = w.x;
  this->view[1][2] = w.y;
  this->view[2][2] = w.z;
  this->view[3][0] = -glm::dot(u, position);
  this->view[3][1] = -glm::dot(v, position);
  this->view[3][2] = -glm::dot(w, position);

  return this->view;
}

/*
glm::mat4 Camera::view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up){
  const glm::vec3 w{glm::normalize(direction)};
  const glm::vec3 u{glm::normalize(glm::cross(w, up))};
  const glm::vec3 v{glm::cross(w, u)};

  this->view = glm::mat4{1.f};
  this->view[0][0] = u.x;
  this->view[1][0] = u.y;
  this->view[2][0] = u.z;
  this->view[0][1] = v.x;
  this->view[1][1] = v.y;
  this->view[2][1] = v.z;
  this->view[0][2] = w.x;
  this->view[1][2] = w.y;
  this->view[2][2] = w.z;
  this->view[3][0] = -glm::dot(u, position);
  this->view[3][1] = -glm::dot(v, position);
  this->view[3][2] = -glm::dot(w, position);

  return this->view;
}


glm::mat4 Camera::view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up){
    return this->view_direction(position, target - position, up);  
}
*/

glm::mat4 Camera::get_projection_view_matrix(){
    return this->projection * this->view;
}


} // namespace velora
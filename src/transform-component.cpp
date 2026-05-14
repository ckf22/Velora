#include "transform-component.hpp"

namespace velora{

TransformComponent::TransformComponent(glm::vec3 _translation, glm::vec3 _scale, 
  glm::vec3 _rotation, glm::vec3 _anchor ) : translation{_translation}, scale{_scale}, rotation{_rotation}, anchor{_anchor} {

    this->update_transform();
}

glm::mat4 TransformComponent::get_transform(bool updated){
    if( updated )
        this->update_transform();

    return this->transform;
}

void TransformComponent::update_transform(){
    // needs to be read from bottom to top due to matrix multiplication
    this->transform = glm::mat4{1.f};

    // scaling is the last step so no large or small numbers increase the impact of 32bit floating point rounding errrors
    this->transform = glm::scale(this->transform, this->scale);

    // translating the object to its original position, then applying the specified translation
    this->transform = glm::translate(this->transform, this->translation + this->anchor);

    // Tait-Bryan Angles y(1) x(2) z(3)
    this->transform = glm::rotate(this->transform, this->rotation.y, {0.f,1.f,0.f});

    this->transform = glm::rotate(this->transform, this->rotation.x, {1.f,0.f,0.f});

    this->transform = glm::rotate(this->transform, this->rotation.z, {0.f,0.f,1.f});

    // translating so the anchor is moved to the origin
    this->transform = glm::translate(this->transform, -this->anchor);
}


}
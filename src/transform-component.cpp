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
    this->transform = glm::mat4{1.f};
    this->transform = glm::translate(this->transform, this->anchor - this->translation);
    this->transform = glm::rotate(this->transform, this->rotation.y, {0.f,1.f,0.f});
    this->transform = glm::rotate(this->transform, this->rotation.x, {1.f,0.f,0.f});
    this->transform = glm::rotate(this->transform, this->rotation.z, {0.f,0.f,1.f});
    this->transform = glm::translate(this->transform, this->translation - this->anchor);
    this->transform = glm::scale(this->transform, this->scale);
}


}
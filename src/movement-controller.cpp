#include "movement-controller.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <limits>

namespace velora{

MovementController::MovementController(Window& _window) : window{_window} {
    this->key_mappings = {
        // moving
        {GLFW_KEY_A, &this->data.position_dn.x},
        {GLFW_KEY_D, &this->data.position_dp.x},
        {GLFW_KEY_W, &this->data.position_dp.z},
        {GLFW_KEY_S, &this->data.position_dn.z},
        {GLFW_KEY_Q, &this->data.position_dp.y},
        {GLFW_KEY_E, &this->data.position_dn.y},

        // turning
        {GLFW_KEY_DOWN, &this->data.rotation_dn.x},
        {GLFW_KEY_UP, &this->data.rotation_dp.x},
        {GLFW_KEY_LEFT, &this->data.rotation_dn.y},
        {GLFW_KEY_RIGHT, &this->data.rotation_dp.y},
    };
}

void MovementController::read_keys(){
    for(auto& it : this->key_mappings){
        if( glfwGetKey(&this->window.get_window(), it.address) == GLFW_PRESS)
            *it.value_ptr += 1.f;
    }
}

void MovementController::apply_to_camera(Camera& camera){
    this->data = InputData{};
    this->read_keys();

    glm::vec3 movement = (this->data.position_dp-this->data.position_dn) * glm::vec3{.08f};
    glm::vec2 turning = ( this->data.rotation_dp-this->data.rotation_dn ) * glm::vec2{.01f};
    turning.y = glm::mod(turning.y + camera.rotation.y, glm::two_pi<float>());
    turning.x = glm::clamp(turning.x + camera.rotation.x, glm::radians(-85.f), glm::radians(85.f)); // limiting up/down camera pitch to 85 degrees

    camera.view_angles(  camera.position + movement,  glm::vec3{turning, .0f} );
}

void MovementController::execute(std::shared_mutex&, std::list<KeyMapping>&){}


}
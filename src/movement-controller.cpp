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
    glfwPollEvents();
    for(auto& it : this->key_mappings){
        if( glfwGetKey(&this->window.get_window(), it.address) == GLFW_PRESS)
            *it.value_ptr += 1.f;
    }
}

void MovementController::apply_to_camera(Camera& camera){
    this->data = InputData{};
    this->read_keys();

    auto new_camera_data = this->refine_input(this->data, camera, glm::radians(85.f), {true,false,true});
    camera.view_angles( new_camera_data.position, new_camera_data.rotation );
}

MovementController::OutputData MovementController::refine_input(InputData& input, Camera& reference, float vertical_tilt_clamp, std::vector<bool> relative_controls){
    glm::vec2 turning = ( this->data.rotation_dp-this->data.rotation_dn ) * glm::vec2{.01f};
    turning.y = glm::mod(turning.y + reference.rotation.y, glm::two_pi<float>());
    turning.x = glm::clamp(turning.x + reference.rotation.x, -vertical_tilt_clamp, vertical_tilt_clamp);

    glm::vec3 movement = (this->data.position_dp-this->data.position_dn) * .08f;

    glm::mat4 rotate{1.f};
    rotate = glm::rotate(rotate, turning.y, {0,1,0});
    rotate = glm::rotate(rotate, turning.x, {1,0,0});

    movement = glm::vec3{ rotate * glm::vec4{movement, 1} };

    OutputData ret{
        reference.position + movement,
        glm::vec3{turning, .0f}
    };
    return ret;
}

/*MovementController::OutputData MovementController::refine_input(InputData& input, Camera& reference, float vertical_tilt_clamp, std::vector<bool> relative_controls){
    // 1. Turning logic remains unchanged
    glm::vec2 turning = ( this->data.rotation_dp-this->data.rotation_dn ) * glm::vec2{.01f};
    turning.y = glm::mod(turning.y + reference.rotation.y, glm::two_pi<float>());
    turning.x = glm::clamp(turning.x + reference.rotation.x, -vertical_tilt_clamp, vertical_tilt_clamp);

    glm::vec3 raw_movement = (this->data.position_dp - this->data.position_dn) * .08f;

    glm::mat4 rotate{1.f};
    rotate = glm::rotate(rotate, turning.y, {0,1,0}); // First: Yaw (around world Up)
    rotate = glm::rotate(rotate, turning.x, {1,0,0}); // Second: Pitch (around local Right)

    glm::vec3 dir_x{1.f, 0.f, 0.f};
    glm::vec3 dir_y{0.f, 1.f, 0.f};
    glm::vec3 dir_z{0.f, 0.f, 1.f};

    if( relative_controls[0] )
        dir_x = glm::vec3(rotate[0]);
    
    if( relative_controls[1] )
        dir_y = glm::vec3(rotate[1]);
    
    if( relative_controls[2] )
        dir_z = glm::vec3(rotate[2]);

    glm::vec3 final_movement = (dir_x * raw_movement.x) + 
                               (dir_y * raw_movement.y) + 
                               (dir_z * raw_movement.z);

    OutputData ret{
        reference.position + final_movement,
        glm::vec3{turning, .0f}
    };
    return ret;
}*/

}
#pragma once

#include "camera.hpp"

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <array>
#include <vector>


namespace velora{

class MovementController{
    typedef unsigned int KeyAddress;
    struct KeyMapping{
        KeyAddress address;
        float * value_ptr;
    };
    struct InputData{
        // 'd' stands for delta/difference; 'n' and 'p' for negative and positive respectively
        glm::vec3 position_dp{.0f}, position_dn{.0f};
        glm::vec2 rotation_dp{.0f}, rotation_dn{.0f};
    };
    struct OutputData{
        glm::vec3 position{.0f};
        glm::vec3 rotation{.0f};
    };

  public:
    MovementController();

    MovementController(const MovementController&) = delete;
    MovementController& operator=(const MovementController&) = delete;

    void apply_to_camera(Camera& camera, GLFWwindow& window);
  private:
    void read_keys(GLFWwindow& window);
    // relative controls mean that the input(e.g. forward) is applied in the perspective of the camera instead
    OutputData refine_input(
        InputData& input, Camera& reference, float vertical_tilt_clamp = 1.4835298642, // 85 degrees
        std::array<bool, 3> relative_controls = {true, false, true}
    );

    InputData data;
    std::vector<KeyMapping> key_mappings;
};

} // namespace velora

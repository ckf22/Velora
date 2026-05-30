#pragma once

#include "window.hpp"
#include "camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

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
    MovementController(Window& _window);

    MovementController(const MovementController&) = delete;
    MovementController& operator=(const MovementController&) = delete;

    void apply_to_camera(Camera& camera);
  private:
    void read_keys();
    // relative controlls mean that the input(e.g. forward) is applied in the perspective of the camera instead
    OutputData refine_input(InputData& input, Camera& reference, float vertical_tilt_clamp = glm::radians(85.f), std::vector<bool> relative_controls = {true, false, true});

    Window& window;

    InputData data;
    std::vector<KeyMapping> key_mappings;
};

} // namespace velora

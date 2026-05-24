#pragma once

#include "window.hpp"
#include "camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <list>


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
    glm::vec3 position;
    glm::vec3 rotation;
};

  public:
    MovementController(Window& _window);

    MovementController(const MovementController&) = delete;
    MovementController& operator=(const MovementController&) = delete;

    void apply_to_camera(Camera& camera);
  private:
    static void execute(std::shared_mutex&, std::list<KeyMapping>&); // used for the thread
    void read_keys();

    Window& window;

    InputData data;
    std::list<KeyMapping> key_mappings;

    std::shared_mutex mutex;
    std::thread thread;
};

} // namespace velora

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>

namespace velora{

struct Vertex{
    glm::vec2 position;
    glm::vec3 color;
    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
    static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();

    static std::vector<Vertex> get_default_triangle(){
      return {
        {.position={0, -0.5},   .color={0.2f,0.2f,1.f}},
        {.position={-0.5, 0.5}, .color={0.2f,1.f,0.2f}},
        {.position={0.5, 0.5},  .color={1.f,0.2f,0.2f}}
      };
      /*
      return {
        {{0,-0.5}},
        {{-0.3535533906, 0.3535533906}},
        {{0.3535533906, 0.3535533906}},
        {{1,1}},
        {{0.8,1}},
        {{1,0.8}}
      };*/
    }
};

class Object{
  public:
    Object();
  //private:
    std::vector<Vertex> data;
};

}
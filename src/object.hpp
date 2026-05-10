#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>

namespace velora{

struct Vertex{
    glm::vec2 position;
    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
    static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();

    static std::vector<Vertex> get_default_triangle(){
      return {
        {{0, -0.5}},
        {{-0.5, 0.5}},
        {{0.5, 0.5}}
      };
    }
};

class Object{
  public:
    Object();
  //private:
    std::vector<Vertex> data;
};

}
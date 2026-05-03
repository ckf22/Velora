#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/matrix.hpp>

#include <vector>

namespace velora{

struct Vertex{
    glm::vec3 position;
    glm::vec3 color{1};
    glm::vec3 normal;
    static std::vector<VkFormat> get_format_layout(){
        return {VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT};
    }
};

class Object{
  public:
    Object();
  private:
    std::vector<Vertex> data;
    std::vector<size_t> indices;

    glm::vec3 position;
    float rx = 0, ry = 0, rz = 0; // radians
    glm::mat4x4 world_space_transformation{1};
};

}
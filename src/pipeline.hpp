#pragma once

#include "device.hpp"
#include <vector>
#include <string>

namespace velora{

class Pipeline{
    struct PipelineConfigInfo{
      VkViewport viewport;
      VkRect2D scissor;
      VkPipelineInputAssemblyStateCreateInfo input_assembly;
      VkPipelineRasterizationStateCreateInfo rasterization;
      VkPipelineMultisampleStateCreateInfo multisample;
      VkPipelineColorBlendAttachmentState color_blend_attachment;
      VkPipelineColorBlendStateCreateInfo color_blend;
      VkPipelineDepthStencilStateCreateInfo depth_stencil;
      VkPipelineRenderingCreateInfo rendering_ci;
    };

  public:
    Pipeline(Device& _device, std::string vertex_filepath, std::string fragment_filepath, VkExtent2D _extent, VkFormat * _image_format, VkFormat& _depth_format);
    ~Pipeline();

    void operator=(const Pipeline&) = delete;
    Pipeline(Pipeline&) = delete;

    void bind_cmd_buffer(VkCommandBuffer& cmd_buffer);
  private:
    void create_pipeline(std::string vertex_filepath, std::string fragment_filepath, VkExtent2D _extent, VkFormat * _image_format, VkFormat& _depth_format);
    void create_pipeline_layout();
    void create_shader_module(VkShaderModule * target_module ,std::string filepath);

    static std::vector<char> read_file(std::string filename);
    static PipelineConfigInfo get_default_config_info(VkExtent2D _extent, VkFormat * _image_format, VkFormat& _depth_format);

    #ifdef DEBUG
    static constexpr bool debug = true;
    #else
    static constexpr bool debug = false;
    #endif

    Device& device;

    VkShaderModule vertex_shader;
    VkShaderModule fragment_shader;

    VkPipelineLayout pipeline_layout{};
    VkPipeline pipeline;
};

}
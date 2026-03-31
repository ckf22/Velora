#pragma once

#include "device.hpp"
#include <string>
#include <vector>

namespace velora {
struct PipelineConfigInfo {
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class Pipeline{
  public:
    Pipeline(Device& _device, const PipelineConfigInfo pipeline_info, std::string vertex_filepath, std::string fragment_filepath);
    ~Pipeline();

    Pipeline& operator=(const Pipeline&) = delete;
    void operator=(Pipeline) = delete;

    void bind(VkCommandBuffer command_buffer);

    static PipelineConfigInfo default_pipeline_config(uint32_t width, uint32_t height);
  private:
    static std::vector<char> read_file(std::string filepath);

    Device& device;
    VkPipeline vk_pipeline;
    VkShaderModule vertex_shader;
    VkShaderModule fragment_shader;

    void create_pipeline(PipelineConfigInfo config_info, std::string vertex_filepath, std::string fragment_filepath);
    void create_shader_module(const std::vector<char>& shader_code, VkShaderModule * module);
};

}
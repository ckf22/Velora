#include "pipeline.hpp"
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace velora {

Pipeline::Pipeline(Device& _device, const PipelineConfigInfo pipeline_info, std::string vertex_filepath, std::string fragment_filepath) : device{_device} {
    this->create_pipeline(pipeline_info, vertex_filepath, fragment_filepath);
}
Pipeline::~Pipeline(){
    vkDestroyShaderModule(this->device.device(), this->vertex_shader, nullptr);
    vkDestroyShaderModule(this->device.device(), this->fragment_shader, nullptr);
    vkDestroyPipeline(this->device.device(), this->vk_pipeline, nullptr);
}

void Pipeline::bind(VkCommandBuffer command_buffer){
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->vk_pipeline);
}

PipelineConfigInfo Pipeline::default_pipeline_config(uint32_t width, uint32_t height){
    PipelineConfigInfo ret{};

    ret.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ret.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    ret.input_assembly_info.primitiveRestartEnable = VK_FALSE;

    ret.viewport.x = 0.0f;
    ret.viewport.y = 0.0f;
    ret.viewport.width = static_cast<float>(width);
    ret.viewport.height = static_cast<float>(height);
    ret.viewport.minDepth = 0.0f;
    ret.viewport.maxDepth = 1.0f;

    ret.scissor.offset = {0, 0};
    ret.scissor.extent = {width, height};

    ret.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    ret.rasterizationInfo.depthClampEnable = VK_FALSE;
    ret.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    ret.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    ret.rasterizationInfo.lineWidth = 1.0f;
    ret.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    ret.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    ret.rasterizationInfo.depthBiasEnable = VK_FALSE;
    ret.rasterizationInfo.depthBiasConstantFactor = 0.0f;   
    ret.rasterizationInfo.depthBiasClamp = 0.0f;            
    ret.rasterizationInfo.depthBiasSlopeFactor = 0.0f;      

    ret.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ret.multisampleInfo.sampleShadingEnable = VK_FALSE;
    ret.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ret.multisampleInfo.minSampleShading = 1.0f;            
    ret.multisampleInfo.pSampleMask = nullptr;              
    ret.multisampleInfo.alphaToCoverageEnable = VK_FALSE;   
    ret.multisampleInfo.alphaToOneEnable = VK_FALSE;        

    ret.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;
    ret.colorBlendAttachment.blendEnable = VK_FALSE;
    ret.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;    
    ret.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;   
    ret.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;               
    ret.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;    
    ret.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;   
    ret.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;               

    ret.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ret.colorBlendInfo.logicOpEnable = VK_FALSE;
    ret.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;   
    ret.colorBlendInfo.attachmentCount = 1;
    ret.colorBlendInfo.pAttachments = &ret.colorBlendAttachment;
    ret.colorBlendInfo.blendConstants[0] = 0.0f;
    ret.colorBlendInfo.blendConstants[1] = 0.0f;
    ret.colorBlendInfo.blendConstants[2] = 0.0f;
    ret.colorBlendInfo.blendConstants[3] = 0.0f;

    ret.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ret.depthStencilInfo.depthTestEnable = VK_TRUE;
    ret.depthStencilInfo.depthWriteEnable = VK_TRUE;
    ret.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    ret.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    ret.depthStencilInfo.minDepthBounds = 0.0f;
    ret.depthStencilInfo.maxDepthBounds = 1.0f;
    ret.depthStencilInfo.stencilTestEnable = VK_FALSE;
    return ret;
}


std::vector<char> Pipeline::read_file(std::string filepath){
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if(!file.is_open())
        throw std::runtime_error("failed to open file: \'" + filepath + '\'');

    size_t size = static_cast<size_t>(file.tellg());

    std::vector<char> output(size);

    file.seekg(0);
    file.read(output.data(), size);

    file.close();

    return output;
}

void Pipeline::create_pipeline(PipelineConfigInfo config_info, std::string vertex_filepath, std::string fragment_filepath){
    assert(config_info.pipelineLayout != VK_NULL_HANDLE && "cannot create  GraphicsPipeline   No layout specified");
    assert(config_info.renderPass != VK_NULL_HANDLE && "cannot create  GraphicsPipeline   No renderPass specified");

    std::vector<char> vert_code = Pipeline::read_file(vertex_filepath);
    std::vector<char> frag_code = Pipeline::read_file(fragment_filepath);

    std::cout << "vert_code: " << vert_code.size() << std::endl;
    std::cout << "frag_code: " << frag_code.size() << std::endl;

    this->create_shader_module(vert_code, &this->vertex_shader);
    this->create_shader_module(frag_code, &this->fragment_shader);

    VkPipelineShaderStageCreateInfo shader_stages[2];

    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = this->vertex_shader;
    shader_stages[0].pName = "main";
    shader_stages[0].flags = 0;
    shader_stages[0].pNext = nullptr;
    shader_stages[0].pSpecializationInfo = nullptr;

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = this->fragment_shader;
    shader_stages[1].pName = "main";
    shader_stages[1].flags = 0;
    shader_stages[1].pNext = nullptr;
    shader_stages[1].pSpecializationInfo = nullptr;

    VkPipelineVertexInputStateCreateInfo vertex_shader_input_info{};
    vertex_shader_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_shader_input_info.vertexAttributeDescriptionCount = 0;
    vertex_shader_input_info.vertexBindingDescriptionCount = 0;
    vertex_shader_input_info.pVertexAttributeDescriptions = nullptr;
    vertex_shader_input_info.pVertexBindingDescriptions = nullptr;

    VkPipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &config_info.viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &config_info.scissor;



    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_shader_input_info;
    pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
    pipeline_info.pViewportState = &viewportInfo;
    pipeline_info.pRasterizationState = &config_info.rasterizationInfo;
    pipeline_info.pMultisampleState = &config_info.multisampleInfo;
    pipeline_info.pColorBlendState = &config_info.colorBlendInfo;
    pipeline_info.pDepthStencilState = &config_info.depthStencilInfo;

    pipeline_info.layout = config_info.pipelineLayout;
    pipeline_info.renderPass = config_info.renderPass;
    pipeline_info.subpass = config_info.subpass;

    pipeline_info.basePipelineIndex = -1;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;


    if(  vkCreateGraphicsPipelines( this->device.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->vk_pipeline )  !=  VK_SUCCESS  ){
        throw std::runtime_error("failed to create graphics pipeline");
    }
}

void  Pipeline::create_shader_module(const std::vector<char>& shader_code, VkShaderModule * module){
    VkShaderModuleCreateInfo create{};
    create.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create.codeSize = shader_code.size();
    create.pCode = reinterpret_cast<const uint32_t*> (shader_code.data());

    if( vkCreateShaderModule(this->device.device(), &create, nullptr, module) != VK_SUCCESS ){
        throw std::runtime_error("failed to create shader module");
    }
}

} // namespace velora

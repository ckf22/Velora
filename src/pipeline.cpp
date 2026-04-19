#include "pipeline.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace velora{

Pipeline::Pipeline(Device& _device, std::string vertex_filepath, std::string fragment_filepath, VkExtent2D _extent, VkFormat * _image_format, VkFormat& _depth_format) : device{_device} {
    this->create_pipeline(vertex_filepath, fragment_filepath, _extent, _image_format, _depth_format);
}

Pipeline::~Pipeline(){
    vkDestroyShaderModule(this->device.get_device(), this->vertex_shader, nullptr);
    vkDestroyShaderModule(this->device.get_device(), this->fragment_shader, nullptr);

    vkDestroyPipelineLayout(this->device.get_device(), this->pipeline_layout, nullptr);
    vkDestroyPipeline(this->device.get_device(), this->pipeline, nullptr);
}

void Pipeline::create_pipeline(std::string vertex_filepath, std::string fragment_filepath, VkExtent2D _extent, VkFormat * _image_format, VkFormat& _depth_format){
    this->create_shader_module(&this->vertex_shader, vertex_filepath);
    this->create_shader_module(&this->fragment_shader, fragment_filepath);

    VkPipelineShaderStageCreateInfo vertex_shader_ci{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = this->vertex_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo fragment_shader_ci{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = this->fragment_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {vertex_shader_ci, fragment_shader_ci};

    auto config_info = get_default_config_info(_extent, _image_format, _depth_format);

    VkPipelineVertexInputStateCreateInfo vertex_input_ci{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    VkPipelineViewportStateCreateInfo viewport_state_ci{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &config_info.viewport,
        .scissorCount = 1,
        .pScissors = &config_info.scissor,
    };

    this->create_pipeline_layout();

    VkGraphicsPipelineCreateInfo pipeline_ci{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &config_info.rendering_ci,
        .stageCount = 2,
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_ci,
        .pInputAssemblyState = &config_info.input_assembly,
        .pViewportState = &viewport_state_ci,
        .pRasterizationState = &config_info.rasterization,
        .pMultisampleState = &config_info.multisample,
        .pDepthStencilState = &config_info.depth_stencil,
        .pColorBlendState = &config_info.color_blend,
        .layout = this->pipeline_layout,
    };

    if( vkCreateGraphicsPipelines(this->device.get_device(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &this->pipeline) != VK_SUCCESS )
        throw std::runtime_error("Failed to create graphics pipeline");
}

void Pipeline::create_pipeline_layout(){
    VkPushConstantRange push{};

    VkPipelineLayoutCreateInfo layout_ci{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    if( vkCreatePipelineLayout(this->device.get_device(), &layout_ci, nullptr, &this->pipeline_layout) != VK_SUCCESS )
        throw std::runtime_error("Failed to create Pipeline Layout");

    if constexpr (debug)
        std::cout << "Pipeline Layout created" << std::endl;
}

void Pipeline::create_shader_module(VkShaderModule * target_module, std::string filepath){
    auto buffer = Pipeline::read_file(filepath);

    // spv code is expected to be formatted as a list of u_int32_t, therefore the length must be divisible by 4
    if( buffer.size() % 4 != 0 )
        throw std::runtime_error(std::string("Compiled shader code out of 4 byte alignment: ")+filepath);

    VkShaderModuleCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = buffer.size(),
        .pCode = reinterpret_cast<u_int32_t*>(buffer.data())
    };

    if( vkCreateShaderModule(this->device.get_device(), &ci, nullptr, target_module) != VK_SUCCESS )
        throw std::runtime_error(std::string("failed to create shader module from file: ")+filepath);

    if constexpr (debug)
        std::cout << "Created Shader Module from file \'" << filepath << '\'' << std::endl;
}

Pipeline::PipelineConfigInfo Pipeline::get_default_config_info(VkExtent2D _extent, VkFormat * _image_format, VkFormat& _depth_format){
    PipelineConfigInfo ret{};

    ret.viewport.x = 0.0f;
    ret.viewport.y = 0.0f;
    ret.viewport.width = static_cast<float>(_extent.width);
    ret.viewport.height = static_cast<float>(_extent.height);
    ret.viewport.minDepth = 0.0f;
    ret.viewport.maxDepth = 1.0f;

    ret.scissor.offset = {0, 0};
    ret.scissor.extent = {_extent.width, _extent.height};

    ret.rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    ret.rasterization.depthClampEnable = VK_FALSE;
    ret.rasterization.rasterizerDiscardEnable = VK_FALSE;
    ret.rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    ret.rasterization.lineWidth = 1.0f;
    ret.rasterization.cullMode = VK_CULL_MODE_NONE;
    ret.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    ret.rasterization.depthBiasEnable = VK_FALSE;
    ret.rasterization.depthBiasConstantFactor = 0.0f;   
    ret.rasterization.depthBiasClamp = 0.0f;            
    ret.rasterization.depthBiasSlopeFactor = 0.0f;      

    ret.multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ret.multisample.sampleShadingEnable = VK_FALSE;
    ret.multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ret.multisample.minSampleShading = 1.0f;            
    ret.multisample.pSampleMask = nullptr;              
    ret.multisample.alphaToCoverageEnable = VK_FALSE;   
    ret.multisample.alphaToOneEnable = VK_FALSE;    

    ret.color_blend_attachment.colorWriteMask = 0xF;
    //ret.color_blend_attachment.colorWriteMask =
    //  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
    //  VK_COLOR_COMPONENT_A_BIT;
    //ret.color_blend_attachment.blendEnable = VK_FALSE;
    //ret.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;    
    //ret.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;   
    //ret.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;               
    //ret.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;    
    //ret.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;   
    //ret.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;               

    ret.color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ret.color_blend.logicOpEnable = VK_FALSE;
    ret.color_blend.logicOp = VK_LOGIC_OP_COPY;   
    ret.color_blend.attachmentCount = 1;
    ret.color_blend.pAttachments = &ret.color_blend_attachment;
    ret.color_blend.blendConstants[0] = 0.0f;
    ret.color_blend.blendConstants[1] = 0.0f;
    ret.color_blend.blendConstants[2] = 0.0f;
    ret.color_blend.blendConstants[3] = 0.0f;

    ret.depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ret.depth_stencil.depthTestEnable = VK_TRUE;
    ret.depth_stencil.depthWriteEnable = VK_TRUE;
    ret.depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    ret.depth_stencil.depthBoundsTestEnable = VK_FALSE;
    ret.depth_stencil.minDepthBounds = 0.0f;
    ret.depth_stencil.maxDepthBounds = 1.0f;
    ret.depth_stencil.stencilTestEnable = VK_FALSE;

    ret.input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ret.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    ret.input_assembly.primitiveRestartEnable = VK_FALSE;

    ret.rendering_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    ret.rendering_ci.colorAttachmentCount = 1;
    ret.rendering_ci.pColorAttachmentFormats = _image_format;
    ret.rendering_ci.depthAttachmentFormat = _depth_format;


    return ret;
}


std::vector<char> Pipeline::read_file(std::string filename){
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if(!file)
        throw std::runtime_error(std::string("failed to read file: ")+filename);

    size_t size = file.tellg();
    file.seekg(0);

    std::vector<char> data(size);
    file.read(data.data(), size);

    file.close();

    return data;
}

} // namespace velora
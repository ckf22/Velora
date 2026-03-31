#include "application.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>

namespace velora {
Application::Application(){
    this->create_pipeline_layout();
    this->create_pipeline();
    this->create_command_buffers();
}

Application::~Application(){
    vkDestroyPipelineLayout(this->device.device(), this->pipeline_layout, nullptr);
}

void Application::run(){
    typedef std::chrono::high_resolution_clock Clock;
    auto t0 = Clock::now();
    
    while(!this->window.should_close()){
        t0 = Clock::now();

        glfwPollEvents();
        this->draw_frame();

        std::chrono::duration<float> time = Clock::now() - t0;
        auto buffer = std::chrono::duration_cast< std::chrono::microseconds >(time).count();
        std::cout << buffer << "microseconds" << std::endl;
    }

    vkDeviceWaitIdle(this->device.device());
}

void Application::create_pipeline_layout(){
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    if(  vkCreatePipelineLayout(this->device.device(), &pipeline_layout_info, nullptr, &this->pipeline_layout )  !=  VK_SUCCESS  ){
        throw std::runtime_error("failed to create pipeline layout");
    }   
}

void Application::create_pipeline(){
    auto pipeline_config = Pipeline::default_pipeline_config(this->swapchain.width(), this->swapchain.height());
    pipeline_config.renderPass = this->swapchain.getRenderPass();
    pipeline_config.pipelineLayout = this->pipeline_layout;
    this->pipeline = std::make_unique<Pipeline>(this->device, pipeline_config,
                 "./shaders/constants-shader.vert.spv","./shaders/constants-shader.frag.spv");
}

void Application::create_command_buffers(){
    this->command_buffers.resize(this->swapchain.imageCount());

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandPool = this->device.getCommandPool();
    info.commandBufferCount = static_cast<uint32_t>(this->command_buffers.size());

    if(  vkAllocateCommandBuffers( this->device.device(), &info, this->command_buffers.data() )  !=  VK_SUCCESS  ){
        throw std::runtime_error("failed to allocate commandbuffers");
    }

    for(unsigned int i = 0; i < this->command_buffers.size(); ++i){
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if( vkBeginCommandBuffer(this->command_buffers[i], &begin_info) != VK_SUCCESS ){
            throw std::runtime_error("failed to begin command buffer at index " + std::to_string(i));
        }


        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = this->swapchain.getRenderPass();
        render_pass_info.framebuffer = this->swapchain.getFrameBuffer(i);

        render_pass_info.renderArea.offset = {0,0};
        render_pass_info.renderArea.extent = this->swapchain.getSwapChainExtent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};

        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(this->command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        this->pipeline.get()->bind(this->command_buffers[i]);

        // the actual magic
        vkCmdDraw(this->command_buffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(this->command_buffers[i]);

        if( vkEndCommandBuffer(this->command_buffers[i]) != VK_SUCCESS ){
            throw std::runtime_error("failed to end command buffer");
        }
    }
}

void Application::draw_frame(){
    uint32_t image_index;
    auto result = this->swapchain.acquireNextImage(&image_index);

    if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR ){
        throw std::runtime_error("failed to aquire next image");
    }

    result = this->swapchain.submitCommandBuffers(&this->command_buffers[image_index], &image_index);
    if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR ){
        throw std::runtime_error("failed to submit command buffers");
    }

}



} // namespace velora

#include "application.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <unistd.h>

namespace velora {
Application::Application(){
    this->create_command_buffers(this->device.get_queue_family());
    this->create_semaphores();

    this->render_system.add_ssbo_descriptor_set(this->descriptor_pool);

    if constexpr (debug)
        std::cout << "\n---------------\nSetup completed\n---------------\n" << std::endl;
}

Application::~Application(){
    vkDestroyCommandPool(this->device.get_device(), this->command_pool, VK_NULL_HANDLE);

    for(auto& it : this->image_ready_semaphores)
        vkDestroySemaphore(this->device.get_device(), it, VK_NULL_HANDLE);

    for(auto& it : this->image_aquired_semaphores)
        vkDestroySemaphore(this->device.get_device(), it, VK_NULL_HANDLE);
}

void Application::run(float fps){
    float frame_time_micro_seconds = 1000000.f / fps;

    auto t0 = std::chrono::high_resolution_clock::now(); // used for frame timing
    auto t_u = t0; // used for updating shader data

    // IMPORTANT: Only use for the image_aquired_semaphores because the call to aquire_next_image changes the index inbeetween signalling and waiting
    int local_semaphore_index = 0;

    u_int32_t frame_count = 0;
    while(!this->window.should_close()){

        this->swapchain.wait_for_active_image_fence();
        if(window.was_window_resized()) this->resize();

        this->swapchain.aquire_next_image(this->image_aquired_semaphores[local_semaphore_index]);
        this->render_system.update_shader_data( std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-t_u) );
        t_u = std::chrono::high_resolution_clock::now();
        this->record_command_buffers();
        this->submit_command_buffers(this->image_aquired_semaphores[local_semaphore_index]);
        this->present_image();
        
        local_semaphore_index = (local_semaphore_index+1) % (int)this->image_aquired_semaphores.size();
        frame_count++;

        glfwPollEvents();

        // filling wait time
        while( std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t0).count() < frame_time_micro_seconds ){
            glfwPollEvents();
            // usleep takes microseconds
            usleep(frame_time_micro_seconds / 10);
        }

        //if constexpr (debug)
        //    std::cout << "Frame " << frame_count << std::endl;

        t0 = std::chrono::high_resolution_clock::now();
    }
    vkDeviceWaitIdle(this->device.get_device());
}

void Application::resize(u_int32_t width, u_int32_t height){
    vkDeviceWaitIdle(this->device.get_device());

    glfwSetWindowSize(&this->window.get_window(), width, height);
    vkDestroySwapchainKHR(this->device.get_device(), this->swapchain.get_swapchain(), VK_NULL_HANDLE);
    this->device.destroy_window_surface();
    this->device.recreate_window_surface();
    this->swapchain.recreate_swapchain({width, height});
    this->render_system.register_resize(width, height);
}

void Application::resize(){
    if( !this->window.was_window_resized() ) return;

    vkDeviceWaitIdle(this->device.get_device());
    vkDestroySwapchainKHR(this->device.get_device(), this->swapchain.get_swapchain(), VK_NULL_HANDLE);
    this->device.destroy_window_surface();
    this->device.recreate_window_surface();
    auto new_extent = window.get_window_extent();
    this->swapchain.recreate_swapchain(new_extent);
    this->render_system.register_resize(new_extent.width, new_extent.height);

    this->window.reset_window_resized_flag();
}

void Application::create_command_buffers(u_int32_t queue_family_index){
    VkCommandPoolCreateInfo command_pool_ci{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_index,
    };

    if( vkCreateCommandPool(this->device.get_device(), &command_pool_ci, VK_NULL_HANDLE, &this->command_pool) != VK_SUCCESS )
        throw std::runtime_error("Failed to create command pool");

    if constexpr (debug)
        std::cout << "Command Pool Created" << std::endl;


    VkCommandBufferAllocateInfo cmd_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = this->command_pool,
        .commandBufferCount = static_cast<u_int32_t>(this->swapchain.get_image_count()),
    };

    this->command_buffers.resize(this->swapchain.get_image_count());
    if( vkAllocateCommandBuffers(this->device.get_device(), &cmd_buffer_allocate_info, this->command_buffers.data()) != VK_SUCCESS )
        throw std::runtime_error("Failed to allocate Command Buffers");

    if constexpr (debug)
        std::cout << "Command Buffers created" << std::endl;
}

void Application::create_semaphores(){
    VkSemaphoreCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    this->image_ready_semaphores.resize(this->swapchain.get_image_count());
    this->image_aquired_semaphores.resize(this->swapchain.get_image_count());

    for(int i = 0; i < this->swapchain.get_image_count(); ++i)
        if( vkCreateSemaphore(this->device.get_device(), &ci, VK_NULL_HANDLE, &this->image_ready_semaphores[i]) != VK_SUCCESS 
         || vkCreateSemaphore(this->device.get_device(), &ci, VK_NULL_HANDLE, &this->image_aquired_semaphores[i]) != VK_SUCCESS )
            throw std::runtime_error(std::string("Failed to create Semaphores at index ")+std::to_string(i));

    if constexpr (debug)
        std::cout << "Semaphores created" << std::endl;
}

void Application::present_image(){
    auto index = this->swapchain.get_current_index();
    u_int32_t buffer = index;
    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &this->image_ready_semaphores[index],
        .swapchainCount = 1,    
        .pSwapchains = &this->swapchain.get_swapchain(),
        .pImageIndices = &buffer
    };
    
    auto result = vkQueuePresentKHR(this->device.get_queue(), &present_info);
    if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
        throw std::runtime_error("Failed to present image");

}

void Application::record_command_buffers(){
    auto index = this->swapchain.get_current_index();
    VkCommandBuffer& cmd_buffer = this->command_buffers[index];

    if( vkResetCommandBuffer(cmd_buffer, 0) != VK_SUCCESS)
        throw std::runtime_error("Failed to reset Command Buffer");


    VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    if( vkBeginCommandBuffer(cmd_buffer, &begin_info) != VK_SUCCESS )
        throw std::runtime_error("Failed to begin Command Buffer");


    VkImageMemoryBarrier2 image_memory_barriers[2] = {
        {    // image
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image = this->swapchain.get_image(index),
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
        },
        {    // depth image
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image = this->swapchain.get_depth_image(index),
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .levelCount = 1, .layerCount = 1 }
        }
    };

    VkDependencyInfo image_barrier_dependency{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 2,
        .pImageMemoryBarriers = &image_memory_barriers[0]
    };
    vkCmdPipelineBarrier2(cmd_buffer, &image_barrier_dependency);


    VkRenderingAttachmentInfo color_attachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = this->swapchain.get_image_view(index),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{ .color = { .1f, .1f, .1f, 1 } }
    };
    VkRenderingAttachmentInfo depth_attachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = this->swapchain.get_depth_image_view(index),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = { .depthStencil = {1.0f,  0} }
    };

    VkRenderingInfo render_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {.extent = this->swapchain.get_current_extent()},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
        .pDepthAttachment = &depth_attachment
    };

    this->render_system.fill_ssbo(cmd_buffer, index);

    vkCmdBeginRendering(cmd_buffer, &render_info);

    VkViewport viewport{
        .width = static_cast<float>(this->swapchain.get_current_extent().width),
        .height = static_cast<float>(this->swapchain.get_current_extent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor{.extent = this->swapchain.get_current_extent()};
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    this->pipeline.bind_cmd_buffer(cmd_buffer);

    this->render_system.fill_command_buffer(cmd_buffer, this->pipeline.get_pipeline_layout(), this->descriptor_pool.get_set(), index);

    vkCmdEndRendering(cmd_buffer);

    VkImageMemoryBarrier2 present_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = this->swapchain.get_image(index),
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    };
    VkDependencyInfo present_barrier_dependency{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &present_barrier
    };

    vkCmdPipelineBarrier2(cmd_buffer, &present_barrier_dependency);

    vkEndCommandBuffer(cmd_buffer);

}

void Application::submit_command_buffers(VkSemaphore& image_aquired_semaphore){
    auto index = this->swapchain.get_current_index();
    VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &image_aquired_semaphore,
        .pWaitDstStageMask = &wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &this->command_buffers[index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &this->image_ready_semaphores[index],
    };
    vkQueueSubmit(this->device.get_queue(), 1, &submit_info, this->swapchain.get_fence(index));
}

} // namespace velora

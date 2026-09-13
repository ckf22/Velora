#include "point-light-system.hpp"


namespace velora{

PointLightSystem::PointLightSystem(std::shared_ptr<Device> _device, const unsigned int _max_point_light_count, const u_int32_t max_frames_in_flight)
 : device{_device}, max_point_light_count{_max_point_light_count}, 
 buffer_segment_count{max_frames_in_flight}, 
 buffer_segment_size{static_cast<u_int32_t>(_max_point_light_count*sizeof(PointLight))} {

    this->data = std::make_unique<MyBuffer>(
        _device, sizeof(PointLight), _max_point_light_count * max_frames_in_flight,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
    );

    this->staging = std::make_unique<MyBuffer>(
        _device, sizeof(PointLight), _max_point_light_count * max_frames_in_flight,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    );

    // '1' is the set qualifier in the shader(l. 23): layout(set = 1, binding = 3, std430)....
    this->descriptors = std::make_unique<Descriptors>(_device, max_frames_in_flight, 1);

    this->descriptors->add_binding(
        VkDescriptorSetLayoutBinding{
            .binding = 3,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        }
    );

    this->descriptors->generate_layout();

    this->descriptors->generate_sets();

    VkDescriptorBufferInfo buffer_info{
        .buffer = this->data->get_buffer(),
        .offset = 0,
        .range = this->buffer_segment_size
    };
    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = 3,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        .pBufferInfo = &buffer_info,
    };
    for(int i = 0; i < max_frames_in_flight; ++i){
        buffer_info.offset = i * this->buffer_segment_size;

        this->descriptors->allocate_descriptor(write, i);
    }
}

PointLightSystem::~PointLightSystem(){}

void PointLightSystem::bind_set(VkCommandBuffer& cmd_buffer, VkPipelineLayout& layout, u_int32_t index){
    this->descriptors->bind_descriptor_set(cmd_buffer, layout, index);
}

void PointLightSystem::upload_to_buffer(VkCommandBuffer& cmd_buffer, u_int32_t target_index){
    auto dest = this->staging->map();

    memcpy(
        // Making sure we copy to the right segment of the buffer
        dest + (this->buffer_segment_size * target_index),
        this->point_lights.data(),
        this->point_lights.size() * sizeof(PointLight)
    );

    this->staging->unmap();


    VkBufferCopy region{
        .srcOffset = this->buffer_segment_size * target_index,
        .dstOffset = this->buffer_segment_size * target_index,
        .size = this->buffer_segment_size,
    };

    vkCmdCopyBuffer(cmd_buffer, this->staging->get_buffer(), this->data->get_buffer(), 1, &region);
}

}
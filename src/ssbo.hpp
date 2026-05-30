#pragma once

#include "buffer.hpp"
#include "descriptors.hpp"

#include <memory>


namespace velora{

class SSBO : MyBuffer{
  public:
    SSBO(Device& _device, u_int32_t _size);
    ~SSBO();

    SSBO& operator=(const SSBO&) = delete;
    SSBO(const SSBO&) = delete;
  private:
    DescriptorPool& descriptor_pool;

    u_int32_t size;

    std::unique_ptr<MyBuffer> staging_buffer;
};

}
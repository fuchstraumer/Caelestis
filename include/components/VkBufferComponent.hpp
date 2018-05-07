#pragma once 
#ifndef VPSK_VK_BUFFER_COMPONENT_HPP
#define VPSK_VK_BUFFER_COMPONENT_HPP
#include "ForwardDecl.hpp"
#include <cstdint>
namespace vpsk {

    struct VkBufferComponent {
        vpr::Buffer* Buffer;
        uint32_t Range;
        uint32_t Offset;
    };

}

#endif //!VPSK_VK_BUFFER_COMPONENT_HPP

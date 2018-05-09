#pragma once
#ifndef VPSK_VIRTUAL_TEXTURE_HPP
#define VPSK_VIRTUAL_TEXTURE_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
namespace vpsk {

    struct VirtualTexturePage {
        VkOffset3D Offset;
        VkExtent3D Extent;
        VkSparseImageMemoryBind Binding{};
        VkDeviceSize Size{ 0 };
        uint32_t mipLevel{ 0 };
        uint32_t Layer{ 0 };
        uint32_t Index{ 0 };
    };

    class VirtualTexture {
        VirtualTexture(const VirtualTexture&) = delete;
        VirtualTexture& operator=(const VirtualTexture&) = delete;
    public:

        VirtualTexture(const vpr::Device* device);

    private:

        VkBindSparseInfo sparseInfo;
        VkSparseImageMemoryBindInfo imageBindInfo;
        VkSparseImageOpaqueMemoryBindInfo imageOpaqueBindInfo;
        
    };

}

#endif //!VPSK_VIRTUAL_TEXTURE_HPP

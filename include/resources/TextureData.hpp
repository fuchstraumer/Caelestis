#pragma once
#ifndef VPSK_TEXTURE_DATA_HPP
#define VPSK_TEXTURE_DATA_HPP
#include <vulkan/vulkan.h>
#include "systems/ResourceLoader.hpp"
#include "ForwardDecl.hpp"

namespace vpsk {

    struct TextureData : ResourceData {
        void Load(const LoadRequest& req) final;
        VkImageCreateInfo ImageInfo;
        VkImageViewCreateInfo ViewInfo;
        void TransferToDevice(VkCommandBuffer cmd);
        void FreeStagingBuffer();
    protected:
        virtual void loadTextureDataFromFile(const std::string& path) = 0;
        virtual void createCopyInformation() = 0;
        virtual void updateImageInfo() = 0;
        virtual void updateViewInfo() = 0;
        virtual VkImageAspectFlags aspect() const noexcept = 0;
        virtual uint32_t mipLevels() const noexcept = 0;
        virtual uint32_t arrayLayers() const noexcept = 0;
        virtual VkImageLayout finalLayout() const noexcept = 0;
        virtual VkFormat format() const noexcept = 0;
        std::unique_ptr<vpr::Buffer> stagingBuffer;
    };

}

#endif //!VPSK_TEXTURE_DATA_HPP
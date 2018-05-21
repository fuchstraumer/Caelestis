#pragma once
#ifndef VPSK_TEXTURE_HPP
#define VPSK_TEXTURE_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "core/LogicalDevice.hpp"
#include "resource/Image.hpp"
#include "resource/Sampler.hpp"
#include "resource/Buffer.hpp"

namespace vpsk {

    class Texture {
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
    public:

        Texture(const vpr::Device* dvc) noexcept;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        virtual ~Texture() = default;

        void LoadFromFile(const char* fname);
        void CreateFromFile(const char* fname);
        void CreateFromBuffer(VkBuffer staging_buffer, VkBufferImageCopy* copies, const size_t num_copies);
        void TransferToDevice(VkCommandBuffer cmd);
        void FreeStagingBuffer();
        const VkImage& Image() const noexcept;
        const VkSampler& Sampler() const noexcept;
        const VkImageView& View() const noexcept;
        const VkDescriptorImageInfo& Descriptor() const noexcept;

        void SetImageInfo(VkImageCreateInfo image_info);
        void SetSamplerInfo(VkSamplerCreateInfo sampler_info);
        void SetViewInfo(VkImageViewCreateInfo view_info);

        void SetSharedSampler(VkSampler handle);

    protected:

        void createImage();
        void createView();
        void createSampler();
        void setDescriptorInfo() const;

        virtual void loadTextureDataFromFile(const char* fname) = 0;
        void finishSetup();
        void scheduleDeviceTransfer();
        virtual void createCopyInformation() = 0;
        virtual void updateImageInfo() = 0;
        virtual void updateViewInfo() = 0;
        virtual void updateSamplerInfo() = 0;
        virtual VkFormat format() const noexcept = 0;
        virtual VkImageAspectFlags aspect() const noexcept = 0;
        virtual VkImageLayout finalLayout() const noexcept = 0;
        virtual uint32_t mipLevels() const noexcept = 0;
        virtual uint32_t arrayLayers() const noexcept = 0;

        const vpr::Device* device;
        VkImageCreateInfo imageInfo;
        VkSamplerCreateInfo samplerInfo;
        VkImageViewCreateInfo viewInfo;
        bool viewInfoSet{ false };
        mutable VkDescriptorImageInfo descriptorInfo;
        mutable bool descriptorInfoSet{ false };
        
        std::weak_ptr<void> backingData;
        std::unique_ptr<vpr::Image> image;
        bool usingSharedSampler{ false };
        std::unique_ptr<vpr::Sampler> samplerUnique;

        VkSampler samplerShared;
        std::unique_ptr<vpr::Buffer> stagingBuffer;
        std::vector<VkBufferImageCopy> copyInfo;

    };

}

#endif //!VPSK_TEXTURE_HPP

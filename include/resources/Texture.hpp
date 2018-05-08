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

    template<typename Derived>
    class Texture {
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
    public:

        Texture(const vpr::Device* dvc) noexcept;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        virtual ~Texture() = default;

        void CreateFromFile(const char* fname);
        void CreateFromBuffer(VkBuffer staging_buffer, VkBufferImageCopy* copies, const size_t num_copies);
        void TransferToDevice(VkCommandBuffer cmd) const;
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

        const vpr::Device* device;
        VkImageCreateInfo imageInfo;
        VkSamplerCreateInfo samplerInfo;
        VkImageViewCreateInfo viewInfo;
        bool viewInfoSet{ false };
        mutable VkDescriptorImageInfo descriptorInfo;
        mutable bool descriptorInfoSet{ false };
 
        std::unique_ptr<vpr::Image> image;
        bool usingSharedSampler{ false };
        std::unique_ptr<vpr::Sampler> samplerUnique;

        VkSampler samplerShared;
        std::unique_ptr<vpr::Buffer> stagingBuffer;
        std::vector<VkBufferImageCopy> copyInfo;
    };

    template<typename Derived>
    inline Texture<Derived>::Texture(const vpr::Device * dvc) noexcept : device(dvc), image(std::make_unique<vpr::Image>(dvc)), samplerUnique(nullptr), samplerShared(VK_NULL_HANDLE) {}

    template<typename Derived>
    inline Texture<Derived>::Texture(Texture && other) noexcept : device(other.device), imageInfo(std::move(other.imageInfo)), samplerInfo(std::move(other.samplerInfo)), viewInfo(std::move(other.viewInfo)),
        viewInfoSet(std::move(other.viewInfoSet)), descriptorInfo(std::move(other.descriptorInfo)), descriptorInfoSet(std::move(other.descriptorInfoSet)), image(std::move(other.image)), sampler(std::move(other.sampler)),
        stagingBuffer(std::move(other.stagingBuffer)), copyInfo(std::move(other.copyInfo)) {}

    template<typename Derived>
    inline Texture<Derived>& Texture<Derived>::operator=(Texture && other) noexcept {
        device = other.device;
        imageInfo = std::move(other.imageInfo);
        samplerInfo = std::move(other.samplerInfo);
        viewInfo = std::move(other.viewInfo);
        viewInfoSet = std::move(other.viewInfoSet);
        descriptorInfo = std::move(other.descriptorInfo);
        descriptorInfoSet = std::move(other.descriptorInfoSet);
        image = std::move(other.image);
        sampler = std::move(other.sampler);
        stagingBuffer = std::move(other.stagingBuffer);
        copyInfo = std::move(other.copyInfo);
        return *this;
    }

    template<typename Derived>
    inline void Texture<Derived>::CreateFromFile(const char * fname) {
        static_cast<Derived*>(this)->loadTextureDataFromFile(fname);
        static_cast<Derived*>(this)->createCopyInformation();
        static_cast<Derived*>(this)->updateImageInfo();
        createImage();
        static_cast<Derived*>(this)->updateViewInfo();
        createView();
        if (!usingSharedSampler) {
            static_cast<Derived*>(this)->updateSamplerInfo();
            createSampler();
        }
    }

    template<typename Derived>
    inline void Texture<Derived>::CreateFromBuffer(VkBuffer staging_buffer, VkBufferImageCopy * copies, const size_t num_copies) {
        copyInfo = std::vector<VkBufferImageCopy>{ copies, copies + num_copies };
        createImage();
        createView();
        if (!usingSharedSampler) {
            createSampler();
        }
    }

    template<typename Derived>
    inline void Texture<Derived>::TransferToDevice(VkCommandBuffer cmd) const {
        auto barrier0 = vpr::Image::GetMemoryBarrier(image->vkHandle(), static_cast<Derived*>(this)->format(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        barrier0.subresourceRange = { static_cast<Derived*>(this)->aspect(), 0, static_cast<Derived*>(this)->mipLevels(), 0, static_cast<Derived*>(this)->layerCount() };
        auto barrier1 = vpr::Image::GetMemoryBarrier(image->vkHandle(), static_cast<Derived*>(this)->format(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<Derived*>(this)->finalLayout());
        barrier1.subresourceRange = barrier0.subresourceRange;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier0);
        vkCmdCopyBufferToImage(cmd, stagingBuffer->vkHandle(), image->vkHandle(), static_cast<Derived*>(this)->finalLayout(), static_cast<uint32_t>(copyInfo.size()), copyInfo.data());
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);
    }

    template<typename Derived>
    inline const VkImage & Texture<Derived>::Image() const noexcept {
        return image->vkHandle();
    }

    template<typename Derived>
    inline const VkSampler& Texture<Derived>::Sampler() const noexcept {
        if (!usingSharedSampler) {
            return samplerUnique->vkHandle();
        }
        else {
            return samplerShared;
        }
    }

    template<typename Derived>
    inline const VkImageView& Texture<Derived>::View() const noexcept {
        if (!viewInfoSet) {
            createImageView();
        }
        return image->View();
    }

    template<typename Derived>
    inline const VkDescriptorImageInfo& Texture<Derived>::Descriptor() const noexcept {
        if (!descriptorInfoSet) {
            setDescriptorInfo();
        }
        return descriptorInfo;
    }

    template<typename Derived>
    inline void Texture<Derived>::SetImageInfo(VkImageCreateInfo image_info) {
        imageInfo = std::move(image_info);
    }

    template<typename Derived>
    inline void Texture<Derived>::SetSamplerInfo(VkSamplerCreateInfo sampler_info) {
        samplerInfo = std::move(sampler_info);
    }

    template<typename Derived>
    inline void Texture<Derived>::SetViewInfo(VkImageViewCreateInfo view_info) {
        viewInfo = std::move(view_info);
    }

    template<typename Derived>
    inline void Texture<Derived>::SetSharedSampler(VkSampler handle) {
        samplerShared = std::move(handle);
        usingSharedSampler = true;
        samplerUnique.reset(); // potentially free a bit of memory
    }

    template<typename Derived>
    inline void Texture<Derived>::createImage() {
        image->Create(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    template<typename Derived>
    inline void Texture<Derived>::createView() {
        image->CreateView(viewInfo);
    }

    template<typename Derived>
    inline void Texture<Derived>::createSampler() {
        samplerUnique = std::make_unique<vpr::Sampler>(device, samplerInfo);
    }

    template<typename Derived>
    inline void Texture<Derived>::setDescriptorInfo() const {
        descriptorInfo = VkDescriptorImageInfo{ Sampler(), View(), static_cast<Derived*>(this)->finalLayout() };
    }

}

#endif //!VPSK_TEXTURE_HPP
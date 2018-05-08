#pragma once
#ifndef VPSK_GLI_TEXTURE_HPP
#define VPSK_GLI_TEXTURE_HPP
#include "Texture.hpp"
#include "gli/gli.hpp"
namespace vpsk {

    template<typename gli_texture_type>
    class gliTexture : public Texture {
    public:

        gliTexture(const vpr::Device* dvc, const char* fname);

        void loadTextureDataFromFile(const char* fname);
        void createCopyInformation();

        void updateImageInfo();
        void updateSamplerInfo();
        constexpr void updateViewInfo();

        constexpr VkImageType type() const;
        constexpr VkImageViewType viewType() const;
        constexpr VkImageCreateFlags flags() const;
        constexpr VkImageLayout finalLayout() const;
        constexpr VkImageAspectFlags aspect() const;
        VkFormat format() const;
        VkExtent3D extent() const;
        gli_texture_type data;

    };

    template<typename gli_texture_type>
    inline gliTexture<gli_texture_type>::gliTexture(const vpr::Device * dvc, const char * fname) : Texture(dvc) {
        createFromFile(fname);
    }

    template<typename gli_texture_type>
    inline void gliTexture<gli_texture_type>::loadTextureDataFromFile(const char * fname) {
        data = gli_texture_type(gli::load(fname));
        stagingBuffer = std::make_unique<vpr::Buffer>(device);
        stagingBuffer->Create(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, static_cast<VkDeviceSize>(data.size()));
        stagingBuffer->CopyToMapped(data.data(), data.size(), 0);
    }

    template<>
    inline void gliTexture<gli::texture2d>::createCopyInformation() {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < data.levels(); ++i) {
            copyInfo.emplace_back(
                VkBufferImageCopy{
                    offset,
                    0,
                    0,
                    VkImageSubresourceLayers{ aspect(), i, 0, static_cast<uint32_t>(data.layers()) },
                    VkOffset3D{ static_cast<uint32_t>(data[i].extent().x), static_cast<uint32_t>(data[i].extent().y), static_cast<uint32_t>(data[i].extent().z) }
                }
            );
            offset += static_cast<uint32_t>(data[i].size());
        }
    }

    template<>
    inline void gliTexture<gli::texture2d_array>::createCopyInformation() {
        uint32_t offset = 0;
        const uint32_t layer_count = static_cast<uint32_t>(data.layers());
        const uint32_t level_count = static_cast<uint32_t>(data.levels());
        for (uint32_t layer = 0; layer < layer_count; ++layer) {
            for (uint32_t level = 0; level < level_count; ++level) {
                copyInfo.emplace_back(
                    VkBufferImageCopy{
                        offset,
                        0,
                        0,
                        VkImageSubresourceLayers{ aspect(), level, layer, 1 },
                        VkOffset3D{ 0, 0, 0 },
                        VkExtent3D{ static_cast<uint32_t>(data[layer][level].extent().x), static_cast<uint32_t>(data[layer][level].extent().y), static_cast<uint32_t>(data[layer][level].extent().z) }
                    }
                );
                offset += static_cast<uint32_t>(data[layer][level].size());
            }
        }
    }

    template<>
    inline void gliTexture<gli::texture_cube>::createCopyInformation() {
        uint32_t offset = 0;
        const uint32_t level_count = static_cast<uint32_t>(data.levels());
        for (uint32_t face_idx = 0; face_idx < 6; ++face_idx) {
            for (uint32_t level = 0; level < level_count; ++level) {
                copyInfo.emplace_back(
                    VkBufferImageCopy{
                        offset, 
                        0,
                        0,
                        VkImageSubresourceLayers{ aspect(), level, face_idx, 1 },
                        VkOffset3D{ 0, 0, 0 },
                        VkExtent3D{ static_cast<uint32_t>(data[face_idx][level].extent().x), static_cast<uint32_t>(data[face_idx][level].extent().y), static_cast<uint32_t>(data[face_idx][level].extent().z) }
                    }
                );
                offset += static_cast<uint32_t>(data[face_idx][level].size());
            }
        }
    }

    template<typename gli_texture_type>
    inline void gliTexture<gli_texture_type>::updateImageInfo() {
        imageInfo = vpr::vk_image_create_info_base;
        imageInfo.flags = flags();
        imageInfo.imageType = type();
        imageInfo.format = format();
        imageInfo.extent = extent();
        imageInfo.mipLevels = static_cast<uint32_t>(data.levels());
        imageInfo.arrayLayers = static_cast<uint32_t>(data.layers());
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = device->GetFormatTiling(format(), VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    template<typename gli_texture_type>
    inline void gliTexture<gli_texture_type>::updateSamplerInfo() {
        samplerInfo = vpr::vk_sampler_create_info_base;
    }

    template<typename gli_texture_type>
    inline constexpr void gliTexture<gli_texture_type>::updateViewInfo() {
        viewInfo = vpr::vk_image_view_create_info_base;
        viewInfo.subresourceRange.aspectMask = aspect();
        viewInfo.subresourceRange.levelCount = static_cast<uint32_t>(data.levels());
        viewInfo.subresourceRange.layerCount = static_cast<uint32_t>(data.layers());
        viewInfo.viewType = viewType();
    }

    template<typename gli_texture_type>
    inline constexpr VkImageType gliTexture<gli_texture_type>::type() const {
        return VK_IMAGE_TYPE_2D;
    }

    template<>
    inline constexpr VkImageViewType gliTexture<gli::texture2d>::viewType() const {
        return VK_IMAGE_VIEW_TYPE_2D;
    }

    template<>
    inline constexpr VkImageViewType gliTexture<gli::texture2d_array>::viewType() const {
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }

    template<>
    inline constexpr VkImageViewType gliTexture<gli::texture_cube>::viewType() const {
        return VK_IMAGE_VIEW_TYPE_CUBE;
    }

    template<>
    inline constexpr VkImageCreateFlags gliTexture<gli::texture_cube>::flags() const {
        return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    template<typename gli_texture_type>
    inline constexpr VkImageLayout gliTexture<gli_texture_type>::finalLayout() const {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    template<typename gli_texture_type>
    inline constexpr VkImageAspectFlags gliTexture<gli_texture_type>::aspect() const {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    template<typename gli_texture_type>
    inline VkFormat gliTexture<gli_texture_type>::format() const {
        // gli format underlying ID should map to a vulkan format 
        return VkFormat(data.format());
    }

    template<typename gli_texture_type>
    inline VkExtent3D gliTexture<gli_texture_type>::extent() const {
        return VkExtent3D{
            static_cast<uint32_t>(data.extent().width),
            static_cast<uint32_t>(data.extent().height),
            static_cast<uint32_t>(data.extent().depth) 
        };
    }

}

#endif // !VPSK_GLI_TEXTURE_HPP

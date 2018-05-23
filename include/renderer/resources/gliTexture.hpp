#pragma once
#ifndef VPSK_GLI_TEXTURE_HPP
#define VPSK_GLI_TEXTURE_HPP
#include "Texture.hpp"
#include "gli/gli.hpp"
#include "renderer/resources/TextureLoadFunctions.hpp"
#include "renderer/RendererCore.hpp"
#include "renderer/systems/ResourceLoader.hpp"
#include "resource/Buffer.hpp"
namespace vpsk {

    template<typename gli_texture_type>
    class gliTexture : public Texture {
    public:

        gliTexture(const vpr::Device* dvc, const char* fname);

        void loadTextureDataFromFile(const char* fname) final;
        void createFromLoadedData(void* data_ptr);
        void createCopyInformation();

        void updateImageInfo() final;
        void updateSamplerInfo() final;
        void updateViewInfo() final;

        uint32_t width() const;
        uint32_t height() const;
        VkFormat format() const noexcept final;
        VkImageLayout finalLayout() const noexcept final;
        VkImageAspectFlags aspect() const noexcept final;
        uint32_t mipLevels() const noexcept final;
        uint32_t arrayLayers() const noexcept final;

        inline constexpr VkImageType type() const noexcept;
        inline constexpr VkImageViewType viewType() const noexcept;
        inline constexpr VkImageCreateFlags flags() const noexcept;
        inline constexpr VkExtent3D extent() const noexcept;

    };

    template<typename gli_texture_type>
    inline gliTexture<gli_texture_type>::gliTexture(const vpr::Device * dvc, const char * fname) : Texture(dvc) {
        CreateFromFile(fname);
    }

    template<typename gli_texture_type>
    inline void gliTexture<gli_texture_type>::loadTextureDataFromFile(const char * fname) {
        auto& loader = ResourceLoader::GetResourceLoader();
        loader.Subscribe("GLI_IMAGE", FactoryFunctor::create<&LoadGLI_Texture>());
        loader.Load("GLI_IMAGE", fname, SignalFunctor::create<gliTexture, &gliTexture::createFromLoadedData>(this));
    }

    template<typename gli_texture_type>
    inline void gliTexture<gli_texture_type>::createFromLoadedData(void* data) {
        backingData = data;
        gli::texture* ptr = reinterpret_cast<gli::texture*>(backingData);
        stagingBuffer = std::make_unique<vpr::Buffer>(RendererCore::GetRenderer().Device());
        stagingBuffer->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, static_cast<VkDeviceSize>(ptr->size()));
        stagingBuffer->CopyToMapped(ptr->data(), ptr->size(), 0);
        finishSetup();
    }

    template<>
    inline void gliTexture<gli::texture2d>::createCopyInformation() {
        gli::texture2d* ptr = reinterpret_cast<gli::texture2d*>(backingData);
        uint32_t offset = 0;
        for (uint32_t i = 0; i < ptr->levels(); ++i) {
            copyInfo.emplace_back(
                VkBufferImageCopy{
                    offset,
                    0,
                    0,
                    VkImageSubresourceLayers{ aspect(), i, 0, static_cast<uint32_t>(ptr->layers()) },
                    VkOffset3D{ 0, 0, 0 },
                    VkExtent3D{ static_cast<uint32_t>((*ptr)[i].extent().x), static_cast<uint32_t>((*ptr)[i].extent().y), static_cast<uint32_t>((*ptr)[i].extent().z) }
                }
            );
            offset += static_cast<uint32_t>((*ptr)[i].size());
        }
    }

    template<>
    inline void gliTexture<gli::texture2d_array>::createCopyInformation() {
        uint32_t offset = 0;
        gli::texture2d_array* ptr = reinterpret_cast<gli::texture2d_array*>(backingData);
        const uint32_t layer_count = static_cast<uint32_t>(ptr->layers());
        const uint32_t level_count = static_cast<uint32_t>(ptr->levels());
        for (uint32_t layer = 0; layer < layer_count; ++layer) {
            for (uint32_t level = 0; level < level_count; ++level) {
                copyInfo.emplace_back(
                    VkBufferImageCopy{
                        offset,
                        0,
                        0,
                        VkImageSubresourceLayers{ aspect(), level, layer, 1 },
                        VkOffset3D{ 0, 0, 0 },
                        VkExtent3D{ static_cast<uint32_t>((*ptr)[layer][level].extent().x), static_cast<uint32_t>((*ptr)[layer][level].extent().y), static_cast<uint32_t>((*ptr)[layer][level].extent().z) }
                    }
                );
                offset += static_cast<uint32_t>((*ptr)[layer][level].size());
            }
        }
    }

    template<>
    inline void gliTexture<gli::texture_cube>::createCopyInformation() {
        uint32_t offset = 0;
        gli::texture_cube* ptr = reinterpret_cast<gli::texture_cube*>(backingData);
        const uint32_t level_count = static_cast<uint32_t>(ptr->levels());
        for (uint32_t face_idx = 0; face_idx < 6; ++face_idx) {
            for (uint32_t level = 0; level < level_count; ++level) {
                copyInfo.emplace_back(
                    VkBufferImageCopy{
                        offset, 
                        0,
                        0,
                        VkImageSubresourceLayers{ aspect(), level, face_idx, 1 },
                        VkOffset3D{ 0, 0, 0 },
                        VkExtent3D{ static_cast<uint32_t>((*ptr)[face_idx][level].extent().x), static_cast<uint32_t>((*ptr)[face_idx][level].extent().y), static_cast<uint32_t>((*ptr)[face_idx][level].extent().z) }
                    }
                );
                offset += static_cast<uint32_t>((*ptr)[face_idx][level].size());
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
        imageInfo.mipLevels = mipLevels();
        imageInfo.arrayLayers = arrayLayers();
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = device->GetFormatTiling(format(), VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    template<typename gli_texture_type>
    inline void gliTexture<gli_texture_type>::updateSamplerInfo() {
        samplerInfo = vpr::vk_sampler_create_info_base;
    }

    template<typename gli_texture_type>
    inline void gliTexture<gli_texture_type>::updateViewInfo() {
        viewInfo = vpr::vk_image_view_create_info_base;
        viewInfo.subresourceRange.aspectMask = aspect();
        viewInfo.subresourceRange.levelCount = mipLevels();
        viewInfo.subresourceRange.layerCount = arrayLayers();
        viewInfo.viewType = viewType();
    }

    template<typename gli_texture_type>
    inline uint32_t gliTexture<gli_texture_type>::width() const {
        gli::texture* ptr = reinterpret_cast<gli::texture*>(backingData);
        return static_cast<uint32_t>(ptr->extent().x);
    }

    template<typename gli_texture_type>
    inline uint32_t gliTexture<gli_texture_type>::height() const {
        gli::texture* ptr = reinterpret_cast<gli::texture*>(backingData);
        return static_cast<uint32_t>(ptr->extent().y);
    }

    template<typename gli_texture_type>
    inline constexpr VkImageType gliTexture<gli_texture_type>::type() const noexcept {
        return VK_IMAGE_TYPE_2D;
    }

    template<>
    inline constexpr VkImageViewType gliTexture<gli::texture2d>::viewType() const noexcept {
        return VK_IMAGE_VIEW_TYPE_2D;
    }

    template<>
    inline constexpr VkImageViewType gliTexture<gli::texture2d_array>::viewType() const noexcept {
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }

    template<>
    inline constexpr VkImageViewType gliTexture<gli::texture_cube>::viewType() const noexcept {
        return VK_IMAGE_VIEW_TYPE_CUBE;
    }

    template<typename gli_texture_type>
    inline constexpr VkImageCreateFlags gliTexture<gli_texture_type>::flags() const noexcept {
        return VkImageCreateFlags(0);
    }

    template<>
    inline constexpr VkImageCreateFlags gliTexture<gli::texture_cube>::flags() const noexcept {
        return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    template<typename gli_texture_type>
    inline constexpr VkExtent3D gliTexture<gli_texture_type>::extent() const noexcept {
        gli::texture* data = reinterpret_cast<gli::texture*>(backingData);
        return VkExtent3D{
            static_cast<uint32_t>(data->extent().x),
            static_cast<uint32_t>(data->extent().y),
            static_cast<uint32_t>(data->extent().z)
        };
    }

    template<typename gli_texture_type>
    inline VkImageLayout gliTexture<gli_texture_type>::finalLayout() const noexcept {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    template<typename gli_texture_type>
    inline VkImageAspectFlags gliTexture<gli_texture_type>::aspect() const noexcept {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    template<typename gli_texture_type>
    inline VkFormat gliTexture<gli_texture_type>::format() const noexcept {
        // gli format underlying ID should map to a vulkan format 
        gli::texture* data = reinterpret_cast<gli::texture*>(backingData);
        return VkFormat(data->format());
    }

    template<typename gli_texture_type>
    inline uint32_t gliTexture<gli_texture_type>::mipLevels() const noexcept {
        gli::texture* data = reinterpret_cast<gli::texture*>(backingData);
        return static_cast<uint32_t>(data->levels());
    }

    template<typename gli_texture_type>
    inline uint32_t gliTexture<gli_texture_type>::arrayLayers() const noexcept {
        gli::texture* data = reinterpret_cast<gli::texture*>(backingData);
        return static_cast<uint32_t>(data->layers());
    }

}

#endif // !VPSK_GLI_TEXTURE_HPP

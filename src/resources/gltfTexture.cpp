#include "resources/gltfTexture.hpp"
#include "tinygltf/tiny_gltf.h"
#include "resource/Buffer.hpp"
#include "command/CommandPool.hpp"
namespace vpsk {

    gltfTexture::gltfTexture(const vpr::Device * dvc, tinygltf::Image & img, VkSampler sampler) : Texture(dvc), image(img) {
        SetSharedSampler(sampler);
        CreateFromFile(nullptr);
    }

    void gltfTexture::copyDataToStaging() {
        std::unique_ptr<uint8_t[]> buffer_memory;
        uint8_t* img_buffer_ptr = nullptr;
        VkDeviceSize buffer_size = 0;

        if (image.component == 3) {
            buffer_size = image.width * image.height * 4;
            buffer_memory = std::make_unique<uint8_t[]>(buffer_size);
            uint8_t* buffer_rgba = buffer_memory.get();
            uint8_t* buffer_rgb = &image.image[0];

            for (size_t i = 0; i < image.width * image.height; ++i) {
                for (size_t j = 0; j < 3; ++j) {
                    buffer_rgba[j] = buffer_rgb[j];
                }
                buffer_rgba += 4;
                buffer_rgb += 3;
            }

            img_buffer_ptr = buffer_memory.get();
        }
        else {
            img_buffer_ptr = &image.image[0];
            buffer_size = image.image.size();
        }

        stagingBuffer = std::make_unique<vpr::Buffer>(device);
        stagingBuffer->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffer_size);
        stagingBuffer->CopyToMapped(img_buffer_ptr, buffer_size, 0);
    }

    void gltfTexture::generateMipMaps() {

    }

    void gltfTexture::loadTextureDataFromFile(const char * fname) {
        copyDataToStaging();
        generateMipMaps();
    }

    void gltfTexture::createCopyInformation() {
        copyInfo.emplace_back(VkBufferImageCopy{ 0, 0, 0, VkImageSubresourceLayers{ aspect(), 0, 0, 1 }, VkOffset3D{ 0, 0, 0 }, VkExtent3D{ static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height), 1 } });
    }

    VkFormat gltfTexture::format() const {
        if (image.component == 1) {
            return VK_FORMAT_R8_UNORM;
        }
        else if (image.component == 2) {
            return VK_FORMAT_R8G8_UNORM;
        }
        else if (image.component >= 3) {
            // We correct 3-channel images into 4 channel images.
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
        else {
            return VK_FORMAT_UNDEFINED;
        }
    }

    void gltfTexture::updateImageInfo() {
        imageInfo = vpr::vk_image_create_info_base;
        imageInfo.format = format();
        imageInfo.tiling = device->GetFormatTiling(format(), VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT);
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    void gltfTexture::updateSamplerInfo() {
        throw std::domain_error("gltfTexture objects should only be using shared sampler objects, not creating their own!");
    }

    void gltfTexture::updateViewInfo() {
        viewInfo = vpr::vk_image_view_create_info_base;
    }

    VkImageLayout gltfTexture::finalLayout() const noexcept {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkImageAspectFlags gltfTexture::aspect() const noexcept {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    uint32_t gltfTexture::mipLevels() const noexcept {
        return 1;
    }

    uint32_t gltfTexture::arrayLayers() const noexcept {
        return 1;
    }


}

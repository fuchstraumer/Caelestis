#include "resources/stbTexture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace vpsk {

    stbTexture::stbTexture(const vpr::Device * dvc, const char * fname) : Texture<stbTexture>(dvc) {
        CreateFromFile(fname);
    }

    void stbTexture::loadTextureDataFromFile(const char * fname) {
        pixels = stbi_load(fname, &x, &y, &channels, 4);
        stagingBuffer = std::make_unique<vpr::Buffer>(device);
        stagingBuffer->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(uint8_t) * x * y * channels);
        stagingBuffer->CopyToMapped(pixels, sizeof(uint8_t) * x * y * channels, 0);
        stbi_image_free(pixels);
    }

    void stbTexture::createCopyInformation() {
        copyInfo.emplace_back(VkBufferImageCopy{ 0, 0, 0, VkImageSubresourceLayers{ this->aspect(), 0, 0, 1 }, VkOffset3D{ 0, 0, 0 }, VkExtent3D{ this->width(), this->height(), 1 } });
    }

    void stbTexture::updateImageInfo() {
        imageInfo.flags = 0;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format();
        imageInfo.extent = VkExtent3D{ width(), height(), 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = device->GetFormatTiling(format(), VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices = nullptr;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    void stbTexture::updateSamplerInfo() {
        samplerInfo = vpr::vk_sampler_create_info_base;
    }

    void stbTexture::updateViewInfo() {
        viewInfo = vpr::vk_image_view_create_info_base;
    }

    uint32_t stbTexture::width() const {
        return static_cast<uint32_t>(x);
    }

    uint32_t stbTexture::height() const {
        return static_cast<uint32_t>(y);
    }

    VkFormat stbTexture::format() const {
        if (channels == 1) {
            return VK_FORMAT_R8_UNORM;
        }
        else if (channels == 2) {
            return VK_FORMAT_R8G8_UNORM;
        }
        else if (channels == 3) {
            return VK_FORMAT_R8G8B8_UNORM;
        }
        else if (channels == 4) {
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
        else {
            return VK_FORMAT_UNDEFINED;
        }
    }

    constexpr VkImageLayout stbTexture::finalLayout() const {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    constexpr VkImageAspectFlags stbTexture::aspect() const {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

}
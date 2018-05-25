#include "resources/stbTexture.hpp"
#include "resources/TextureLoadFunctions.hpp"
#include "systems/ResourceLoader.hpp"
#include "RendererCore.hpp"
namespace vpsk {

    stbTexture::stbTexture(const vpr::Device * dvc, const char * fname) : Texture(dvc) {
        CreateFromFile(fname);
    }

    void stbTexture::loadTextureDataFromFile(const char * fname) {
        auto& loader = ResourceLoader::GetResourceLoader();
        loader.Subscribe("STB_IMAGE", FactoryFunctor::create<&LoadSTB_Image>());
        loader.Load("STB_IMAGE", fname, SignalFunctor::create<stbTexture, &stbTexture::createFromLoadedData>(this));
    }

    void stbTexture::createFromLoadedData(void* data_ptr) {
        backingData = data_ptr;
        vpr::Device* device_ptr = RendererCore::GetRenderer().Device();
        stbi_image_data_t* ptr = reinterpret_cast<stbi_image_data_t*>(backingData);
        stagingBuffer = std::make_unique<vpr::Buffer>(device_ptr);
        stagingBuffer->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(uint8_t) * ptr->x * ptr->y * ptr->channels);
        stagingBuffer->CopyToMapped(ptr->pixels, sizeof(uint8_t) * ptr->x * ptr->y * ptr->channels, 0);
        finishSetup();
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
        stbi_image_data_t* ptr = reinterpret_cast<stbi_image_data_t*>(backingData);
        return static_cast<uint32_t>(ptr->x);
    }

    uint32_t stbTexture::height() const {
        stbi_image_data_t* ptr = reinterpret_cast<stbi_image_data_t*>(backingData);
        return static_cast<uint32_t>(ptr->y);
    }

    VkFormat stbTexture::format() const noexcept {
        stbi_image_data_t* ptr = reinterpret_cast<stbi_image_data_t*>(backingData);

        if (ptr->channels == 1) {
            return VK_FORMAT_R8_UNORM;
        }
        else if (ptr->channels == 2) {
            return VK_FORMAT_R8G8_UNORM;
        }
        else if (ptr->channels == 3) {
            return VK_FORMAT_R8G8B8_UNORM;
        }
        else if (ptr->channels == 4) {
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
        else {
            return VK_FORMAT_UNDEFINED;
        }
    }

    uint32_t stbTexture::mipLevels() const noexcept {
        return 1;
    }

    uint32_t stbTexture::arrayLayers() const noexcept {
        return 1;
    }

    VkImageLayout stbTexture::finalLayout() const noexcept {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkImageAspectFlags stbTexture::aspect() const noexcept {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

}
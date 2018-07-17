#include "VulkanComplexScene.hpp"
#include "vpr/PipelineCache.hpp"
#include "vpr/DescriptorPool.hpp"
#include "vpr/DescriptorSet.hpp"
#include "vpr/DescriptorSetLayout.hpp"
#include "vpr/PipelineLayout.hpp"
#include "vpr/ShaderModule.hpp"
#include "vpr/LogicalDevice.hpp"
#include "vpr/Swapchain.hpp"
#include "ObjModel.hpp"
#include "../../../plugins/resource_context/include/ResourceTypes.hpp"
#include "../../../plugins/resource_context/include/ResourceContextAPI.hpp"
#include "../../../plugins/resource_context/include/TransferSystem.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "gli/gli.hpp"
#include <iostream>
#include "vpr/vkAssert.hpp"

struct stb_image_data_t {
    stb_image_data_t(const char* fname) {
        pixels = stbi_load(fname, &width, &height, &channels, 4);
        if (!pixels) {
            throw std::runtime_error("Invalid file path for stb_load");
        }
    }
    ~stb_image_data_t() {
        if (pixels) {
            stbi_image_free(pixels);
        }
        pixels = nullptr;
    }
    stbi_uc* pixels = nullptr;
    int width = -1;
    int height = -1;
    int channels = -1;
};

VulkanComplexScene::VulkanComplexScene() : VulkanScene(), skyboxTextureReady(false), houseTextureReady(false), houseMeshReady(false) {}

VulkanComplexScene::~VulkanComplexScene()
{
}

VulkanComplexScene& VulkanComplexScene::GetScene() {
    static VulkanComplexScene scene;
    return scene;
}

void VulkanComplexScene::Construct(RequiredVprObjects objects, void * user_data) {
    vprObjects = objects;
    resourceContext = reinterpret_cast<const ResourceContext_API*>(user_data);
}

void VulkanComplexScene::Destroy() {

}

void* VulkanComplexScene::LoadObjFile(const char* fname) {
    return new LoadedObjModel(fname);
}

void VulkanComplexScene::DestroyObjFileData(void * obj_file) {
    LoadedObjModel* model = reinterpret_cast<LoadedObjModel*>(obj_file);
    delete model;
}

void* VulkanComplexScene::LoadJpegImage(const char* fname) {
    return new stb_image_data_t(fname);
}

void VulkanComplexScene::DestroyJpegFileData(void * jpeg_file) {
    stb_image_data_t* image = reinterpret_cast<stb_image_data_t*>(jpeg_file);
    delete image;
}

void* VulkanComplexScene::LoadCompressedTexture(const char* fname) {
    return new gli::texture_cube(gli::load(fname));
}

void VulkanComplexScene::DestroyCompressedTextureData(void * compressed_texture) {
    gli::texture_cube* texture = reinterpret_cast<gli::texture_cube*>(compressed_texture);
    delete texture;
}

void VulkanComplexScene::CreateHouseMesh(void * obj_data) {
    LoadedObjModel* obj_model = reinterpret_cast<LoadedObjModel*>(obj_data);

    VkBufferCreateInfo vbo_info {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        static_cast<VkDeviceSize>(sizeof(glm::vec3) * obj_model->Vertices.positions.size()),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    // Test multi-copy functionality of resource context, by merging these two 
    // separate data buffers into a single VkBuffer
    const gpu_resource_data_t vbo_data[2] {
        gpu_resource_data_t {
            obj_model->Vertices.positions.data(),
            sizeof(glm::vec3) * obj_model->Vertices.positions.size(),
            0,
            0,
            0
        },
        gpu_resource_data_t {
            obj_model->Vertices.uvs.data(),
            sizeof(glm::vec2) * obj_model->Vertices.uvs.size(),
            0,
            0,
            0
        }
    };

    const VkBufferCreateInfo ebo_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        static_cast<VkDeviceSize>(sizeof(uint32_t) * obj_model->Indices.size()),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    const gpu_resource_data_t ebo_data{
        obj_model->Indices.data(),
        static_cast<size_t>(ebo_info.size),
        0,
        0,
        0
    };

    houseVBO0 = resourceContext->CreateBuffer(&vbo_info, nullptr, 1, vbo_data, uint32_t(memory_type::DEVICE_LOCAL), nullptr);
    vbo_info.size = sizeof(glm::vec2) * obj_model->Vertices.uvs.size();
    houseVBO1 = resourceContext->CreateBuffer(&vbo_info, nullptr, 1, &vbo_data[1], uint32_t(memory_type::DEVICE_LOCAL), nullptr);
    houseEBO = resourceContext->CreateBuffer(&ebo_info, nullptr, 1, &ebo_data, uint32_t(memory_type::DEVICE_LOCAL), nullptr);

    obj_model->Vertices.positions.shrink_to_fit();
    obj_model->Vertices.uvs.shrink_to_fit();
    obj_model->Indices.shrink_to_fit();
    houseMeshReady = true;
}

void VulkanComplexScene::CreateHouseTexture(void * texture_data) {
    stb_image_data_t* image_data = reinterpret_cast<stb_image_data_t*>(texture_data);

    const VkImageCreateInfo image_info{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM,
        VkExtent3D{ static_cast<uint32_t>(image_data->width), static_cast<uint32_t>(image_data->height), 1 },
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        vprObjects.device->GetFormatTiling(VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    const VkImageViewCreateInfo view_info{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        VK_NULL_HANDLE,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM,
        VkComponentMapping{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    };

    const gpu_image_resource_data_t initial_texture_data[1] {
        gpu_image_resource_data_t {
            image_data->pixels,
            sizeof(stbi_uc) * image_data->width * image_data->height * image_data->channels,
            static_cast<uint32_t>(image_data->width), 
            static_cast<uint32_t>(image_data->height),
        }
    };

    houseTexture = resourceContext->CreateImage(&image_info, &view_info, 1, initial_texture_data, uint32_t(memory_type::DEVICE_LOCAL), nullptr);
    houseTextureReady = true;
}

void VulkanComplexScene::CreateSkyboxTexture(void * texture_data) {

    gli::texture_cube* texture = reinterpret_cast<gli::texture_cube*>(texture_data);
    const uint32_t width = static_cast<uint32_t>(texture->extent().x);
    const uint32_t height = static_cast<uint32_t>(texture->extent().y);
    const uint32_t mipLevels = static_cast<uint32_t>(texture->levels());

    const VkImageCreateInfo image_info {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        VK_IMAGE_TYPE_2D,
        (VkFormat)texture->format(),
        VkExtent3D{ width, height, 1 },
        mipLevels,
        6,
        VK_SAMPLE_COUNT_1_BIT,
        vprObjects.device->GetFormatTiling((VkFormat)texture->format(), VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    const VkImageViewCreateInfo view_info{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        VK_NULL_HANDLE,
        VK_IMAGE_VIEW_TYPE_CUBE,
        (VkFormat)texture->format(),
        VkComponentMapping{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 6 }
    };

    std::vector<gpu_image_resource_data_t> image_copies;
    for (size_t i = 0; i < 6; ++i) {
        for (size_t j = 0; j < texture->levels(); ++j) {
            auto& ref = *texture;
            image_copies.emplace_back(gpu_image_resource_data_t{
                ref[i][j].data(),
                ref[i][j].size(),
                static_cast<uint32_t>(ref[i][j].extent().x),
                static_cast<uint32_t>(ref[i][j].extent().y),
                static_cast<uint32_t>(i),
                uint32_t(1),
                static_cast<uint32_t>(j)
            });
        }
    }

    skyboxTexture = resourceContext->CreateImage(&image_info, &view_info, image_copies.size(), image_copies.data(), uint32_t(memory_type::DEVICE_LOCAL), nullptr);
    skyboxTextureReady = true;

}

void VulkanComplexScene::WaitForAllLoaded() {
    while (!skyboxTextureReady || !houseMeshReady || !houseTextureReady) {
        
    }
    resourceContext->CompletePendingTransfers();
    resourceContext->FlushStagingBuffers();
    std::cerr << "All data loaded.";
}

void VulkanComplexScene::update() {
}

void VulkanComplexScene::recordCommands() {
}

void VulkanComplexScene::draw() {
}

void VulkanComplexScene::endFrame() {
}

void VulkanComplexScene::createSampler() {
    constexpr static VkSamplerCreateInfo sampler_info{
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        nullptr,
        0,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        1.0f,
        VK_TRUE,
        4.0f,
        VK_FALSE,
        VK_COMPARE_OP_BEGIN_RANGE,
        0.0f,
        3.0f,
        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        VK_FALSE
    };
    sampler = resourceContext->CreateSampler(&sampler_info, nullptr);
}

void VulkanComplexScene::createUBO() {
    constexpr static VkBufferCreateInfo ubo_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        sizeof(ubo_data_t),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };
    sharedUBO = resourceContext->CreateBuffer(&ubo_info, nullptr, 0, nullptr, uint32_t(memory_type::HOST_VISIBLE_AND_COHERENT), nullptr);
}

void VulkanComplexScene::createDescriptorPool() {
    descriptorPool = std::make_unique<vpr::DescriptorPool>(vprObjects.device->vkHandle(), 3);
    descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
    descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2);
    descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_SAMPLER, 1);
    descriptorPool->Create();
}

void VulkanComplexScene::createDescriptorSetLayouts() {

    constexpr static VkDescriptorSetLayoutBinding base_set_bindings[2] {
        VkDescriptorSetLayoutBinding{
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr
        },
        VkDescriptorSetLayoutBinding{
            1,
            VK_DESCRIPTOR_TYPE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        }
    };

    baseSetLayout = std::make_unique<vpr::DescriptorSetLayout>(vprObjects.device->vkHandle());
    baseSetLayout->AddDescriptorBindings(2, base_set_bindings);

    constexpr static VkDescriptorSetLayoutBinding texture_binding{
        0,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };

    textureSetLayout = std::make_unique<vpr::DescriptorSetLayout>(vprObjects.device->vkHandle());
    textureSetLayout->AddDescriptorBinding(texture_binding);

}

void VulkanComplexScene::createBaseDescriptorSet() {
    baseSet = std::make_unique<vpr::DescriptorSet>(vprObjects.device->vkHandle());
    const VkDescriptorBufferInfo ubo_info{

    };

}

void VulkanComplexScene::createPipelineLayouts() {
    
    const VkDescriptorSetLayout set_layouts[2]{ baseSetLayout->vkHandle(), textureSetLayout->vkHandle() };

    pipelineLayout = std::make_unique<vpr::PipelineLayout>(vprObjects.device->vkHandle());
    pipelineLayout->Create(set_layouts, 2);

}

void VulkanComplexScene::createShaders() {
    houseVert = std::make_unique<vpr::ShaderModule>(vprObjects.device->vkHandle(), "../../../../tests/plugin_tests/ResourceContextSceneTest/House.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    houseFrag = std::make_unique<vpr::ShaderModule>(vprObjects.device->vkHandle(), "../../../../tests/plugin_tests/ResourceContextSceneTest/House.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    skyboxVert = std::make_unique<vpr::ShaderModule>(vprObjects.device->vkHandle(), "../../../../tests/plugin_tests/ResourceContextSceneTest/Skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    skyboxFrag = std::make_unique<vpr::ShaderModule>(vprObjects.device->vkHandle(), "../../../../tests/plugin_tets/ResourceContextSceneTest/Skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void VulkanComplexScene::createFramebuffers() {
    framebuffers.resize(vprObjects.swapchain->ImageCount());
    for (size_t i = 0; i < framebuffers.size(); ++i) {
        std::array<VkImageView, 2> views{
            vprObjects.swapchain->ImageView(i),
            depthStencil.View
        };

        const VkFramebufferCreateInfo create_info{
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            0,
            renderPass,
            static_cast<uint32_t>(views.size()),
            views.data(),
            vprObjects.swapchain->Extent().width,
            vprObjects.swapchain->Extent().height,
            1
        };

        VkResult result = vkCreateFramebuffer(vprObjects.device->vkHandle(), &create_info, nullptr, &framebuffers[i]);
        VkAssert(result);
    }
}

void VulkanComplexScene::createRenderpass() {
    renderPass = CreateBasicRenderpass(vprObjects.device, vprObjects.swapchain, depthStencil.Format);
}

void VulkanComplexScene::createHousePipeline() {

}



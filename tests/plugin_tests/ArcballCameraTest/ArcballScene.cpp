#include "ArcballScene.hpp"
#include "vpr/Semaphore.hpp"
#include "vpr/CommandPool.hpp"
#include "vpr/LogicalDevice.hpp"
#include "vpr/Swapchain.hpp"
#include "vpr/DescriptorPool.hpp"
#include "vpr/DescriptorSet.hpp"
#include "vpr/DescriptorSetLayout.hpp"
#include "vpr/PipelineLayout.hpp"
#include "vpr/PipelineCache.hpp"
#include "vpr/ShaderModule.hpp"
#include "vpr/PhysicalDevice.hpp"
#include "vpr/Fence.hpp"
#include "vpr/Framebuffer.hpp"
#include "vkAssert.hpp"
#include "CommonCreationFunctions.hpp"
#include "resource_context/include/ResourceContextAPI.hpp"
#include "resource_context/include/ResourceTypes.hpp"
#include "renderer_context/include/RendererContextAPI.hpp"
#include "renderer_context/include/core/RendererContext.hpp"
#include "Camera.hpp"
#include "ArcballCameraController.hpp"
#include "ArcballHelper.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "ObjModel.hpp"
#include "ImGuiWrapper.hpp"
#include <array>
#include "glm/gtc/matrix_transform.hpp"

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

ArcballScene::ArcballScene() : VulkanScene() {}

ArcballScene::~ArcballScene() {
    Destroy();
}

ArcballScene& ArcballScene::GetScene() {
    static ArcballScene scene;
    return scene;
}

void ArcballScene::Construct(RequiredVprObjects objects, void * user_data) {
    RequiredInitialData* initial_data = reinterpret_cast<RequiredInitialData*>(user_data);
    resourceApi = initial_data->resourceAPI;
    gui = initial_data->gui;
    camera = initial_data->camera;
    camera->LookAt(glm::vec3(0.0f));
    vprObjects = objects;
    createSemaphores();
    createSampler();
    createHouseUBO();
    createFences();
    createCmdPool();
    createDescriptors();
    createLayouts();
    createDepthStencil();
    createShaders();
    createRenderpass();
    gui->Construct(initial_data->rendererAPI, initial_data->resourceAPI, renderPass);
    auto& helper = ArcballHelper::GetArcballHelper();
    helper.Construct(initial_data->rendererAPI, resourceApi, renderPass);
    createFramebuffers();
    createPipeline();
}

void ArcballScene::Destroy() {
    gui->Destroy();
    auto& helper = ArcballHelper::GetArcballHelper();
    helper.Destroy();
    resourceApi->DestroyResource(sampler);
    resourceApi->DestroyResource(houseEBO);
    resourceApi->DestroyResource(houseVBO);
    resourceApi->DestroyResource(houseUBO);
    resourceApi->DestroyResource(houseTexture);
    cmdPool.reset();
    descriptorSet.reset();
    setLayout.reset();
    descriptorPool.reset();
    vert.reset();
    frag.reset();
    imageAcquireSemaphore.reset();
    renderCompleteSemaphore.reset();
    destroyFences();
    destroyFramebuffers();
    pipelineLayout.reset();
    pipelineCache.reset();
    vkDestroyPipeline(vprObjects.device->vkHandle(), pipeline, nullptr);
    vkDestroyRenderPass(vprObjects.device->vkHandle(), renderPass, nullptr);
    delete depthStencil;
    houseTextureReady = false;
    houseMeshReady = false;
}

void* ArcballScene::LoadObj(const char* fname, void* user_data) {
    return new LoadedObjModel(fname);
}

void ArcballScene::DestroyObj(void* obj_file) {
    LoadedObjModel* model = reinterpret_cast<LoadedObjModel*>(obj_file);
    delete model;
}

void* ArcballScene::LoadPNG(const char* fname, void* user_data) {
    return new stb_image_data_t(fname);
}

void ArcballScene::DestroyPNG(void* jpeg_data) {
    stb_image_data_t* data = reinterpret_cast<stb_image_data_t*>(jpeg_data);
    delete data;
}

void ArcballScene::CreateHouseMesh(void * obj_data) {
    LoadedObjModel* obj_model = reinterpret_cast<LoadedObjModel*>(obj_data);

    VkBufferCreateInfo vbo_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        static_cast<VkDeviceSize>(sizeof(LoadedObjModel::vertex_t) * obj_model->vertices.size()),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    const gpu_resource_data_t vbo_data{
        obj_model->vertices.data(),
        sizeof(LoadedObjModel::vertex_t) * obj_model->vertices.size(),
        0,
        0,
        0
    };

    const VkBufferCreateInfo ebo_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        static_cast<VkDeviceSize>(sizeof(uint32_t) * obj_model->indices.size()),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    const gpu_resource_data_t ebo_data{
        obj_model->indices.data(),
        static_cast<size_t>(ebo_info.size),
        0,
        0,
        0
    };

    houseVBO = resourceApi->CreateBuffer(&vbo_info, nullptr, 1, &vbo_data, uint32_t(memory_type::DEVICE_LOCAL), nullptr);
    houseEBO = resourceApi->CreateBuffer(&ebo_info, nullptr, 1, &ebo_data, uint32_t(memory_type::DEVICE_LOCAL), nullptr);
    houseIndexCount = static_cast<uint32_t>(obj_model->indices.size());

    obj_model->vertices.clear(); obj_model->vertices.shrink_to_fit();
    obj_model->indices.clear(); obj_model->indices.shrink_to_fit();
    houseMeshReady = true;
    updateDescriptorSets();
}

void ArcballScene::CreateHouseTexture(void * texture_data) {
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

    const gpu_image_resource_data_t initial_texture_data[1]{
        gpu_image_resource_data_t{
        image_data->pixels,
        sizeof(stbi_uc) * image_data->width * image_data->height * image_data->channels,
        static_cast<uint32_t>(image_data->width),
        static_cast<uint32_t>(image_data->height),
    }
    };

    houseTexture = resourceApi->CreateImage(&image_info, &view_info, 1, initial_texture_data, uint32_t(memory_type::DEVICE_LOCAL), nullptr);
    houseTextureReady = true;

}

void ArcballScene::UpdateHouseModelMatrix(glm::mat4 model_matrix) {
    uboData.model = std::move(model_matrix);
}

void ArcballScene::update() {
    uboData.projection = camera->ProjectionMatrix();
    //uboData.view = glm::lookAt(glm::vec3(-5.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    uboData.view = camera->ViewMatrix();
    const gpu_resource_data_t data{
        &uboData,
        sizeof(uboData),
        0,
        0,
        0
    };
    resourceApi->SetBufferData(houseUBO, 1, &data);
}

void ArcballScene::recordCommands() {
    static std::array<bool, 5> first_frame{ true, true, true, true, true };

    if (!first_frame[currentBuffer]) {
        cmdPool->ResetCmdBuffer(currentBuffer);
    }

    constexpr static VkCommandBufferBeginInfo begin_info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        nullptr
    };

    constexpr static std::array<VkClearValue, 2> clearValues{
        VkClearValue{ VkClearColorValue{ 0.0f, 0.0f, 0.5f, 1.0f } },
        VkClearValue{ 1.0f, 0 }
    };

    const VkRect2D render_area{
        VkOffset2D{ 0, 0 },
        VkExtent2D{ vprObjects.swapchain->Extent() }
    };

    const VkViewport viewport{
        0.0f,
        0.0f,
        static_cast<float>(vprObjects.swapchain->Extent().width),
        static_cast<float>(vprObjects.swapchain->Extent().height),
        0.0f,
        1.0f
    };

    const VkRect2D scissor{
        render_area
    };

    VkRenderPassBeginInfo rpBegin{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        renderPass,
        VK_NULL_HANDLE,
        render_area,
        static_cast<uint32_t>(clearValues.size()),
        clearValues.data()
    };

    {
        rpBegin.framebuffer = framebuffers[currentBuffer]->vkHandle();
        VkResult result = VK_SUCCESS;
        auto& pool = *cmdPool;
        result = vkBeginCommandBuffer(pool[currentBuffer], &begin_info); VkAssert(result);
        vkCmdBeginRenderPass(pool[currentBuffer], &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
        //gui->DrawFrame(static_cast<size_t>(currentBuffer), pool[currentBuffer]);
        if (houseTextureReady && houseMeshReady) {
            // no sense in rendering helper before it's ready
            vkCmdSetViewport(pool[currentBuffer], 0, 1, &viewport);
            vkCmdSetScissor(pool[currentBuffer], 0, 1, &scissor);
            auto& helper = ArcballHelper::GetArcballHelper();
            const glm::mat4 proj_view = camera->ProjectionMatrix() * camera->ViewMatrix();
            helper.Render(proj_view, pool[currentBuffer]);
            renderHouse(pool[currentBuffer]);
        }
        vkCmdEndRenderPass(pool[currentBuffer]);
        result = vkEndCommandBuffer(pool[currentBuffer]);
        VkAssert(result);
    }

    first_frame[currentBuffer] = false;
}

void ArcballScene::renderHouse(VkCommandBuffer cmd) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->vkHandle(), 0, 1, &descriptorSet->vkHandle(), 0, nullptr);
    const VkBuffer buffers[1]{ (VkBuffer)houseVBO->Handle };
    constexpr static VkDeviceSize offsets[1]{ 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(cmd, (VkBuffer)houseEBO->Handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, houseIndexCount, 1, 0, 0, 0);
}

void ArcballScene::draw() {

    constexpr static VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit_info{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        1,
        &imageAcquireSemaphore->vkHandle(),
        &wait_mask,
        1,
        &cmdPool->GetCmdBuffer(currentBuffer),
        1,
        &renderCompleteSemaphore->vkHandle()
    };

    VkResult result = vkQueueSubmit(vprObjects.device->GraphicsQueue(), 1, &submit_info, fences[currentBuffer]->vkHandle());
    VkAssert(result);
}

void ArcballScene::endFrame() {

    VkResult result = vkWaitForFences(vprObjects.device->vkHandle(), 1, &fences[currentBuffer]->vkHandle(), VK_TRUE, UINT64_MAX);
    VkAssert(result);
    result = vkResetFences(vprObjects.device->vkHandle(), 1, &fences[currentBuffer]->vkHandle());
    VkAssert(result);

}

void ArcballScene::createCmdPool() {
    const VkCommandPoolCreateInfo pool_info{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        vprObjects.device->QueueFamilyIndices.Graphics
    };
    cmdPool = std::make_unique<vpr::CommandPool>(vprObjects.device->vkHandle(), pool_info);
    cmdPool->AllocateCmdBuffers(vprObjects.swapchain->ImageCount(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void ArcballScene::createSampler() {
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
    sampler = resourceApi->CreateSampler(&sampler_info, nullptr);
}

void ArcballScene::createFences() {
    fences.resize(vprObjects.swapchain->ImageCount());

    for (auto& handle : fences) {
        handle = std::make_unique<vpr::Fence>(vprObjects.device->vkHandle(), 0);
    }

}

void ArcballScene::destroyFences() {
    for (auto& fence : fences) {
        fence.reset();
    }
}

void ArcballScene::createDepthStencil() {
    depthStencil = new DepthStencil();
    *depthStencil = CreateDepthStencil(vprObjects.device, vprObjects.physicalDevice, vprObjects.swapchain);
}

void ArcballScene::createFramebuffers() {
    framebuffers.resize(vprObjects.swapchain->ImageCount());

    for (size_t i = 0; i < framebuffers.size(); ++i) {

        std::array<VkImageView, 2> views{
            vprObjects.swapchain->ImageView(i),
            depthStencil->View
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

        framebuffers[i] = std::make_unique<vpr::Framebuffer>(vprObjects.device->vkHandle(), create_info);

    }
}

void ArcballScene::destroyFramebuffers() {
    for (auto& fbuff : framebuffers) {
        fbuff.reset();
    }
}

void ArcballScene::createDescriptors() {
    descriptorPool = std::make_unique<vpr::DescriptorPool>(vprObjects.device->vkHandle(), 3);
    descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
    descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2);
    descriptorPool->Create();
    descriptorSet = std::make_unique<vpr::DescriptorSet>(vprObjects.device->vkHandle());
}

void ArcballScene::updateDescriptorSets() {
    descriptorSet->AddDescriptorInfo(VkDescriptorBufferInfo{ (VkBuffer)houseUBO->Handle, 0, sizeof(ubo_data_t) }, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    descriptorSet->AddDescriptorInfo(VkDescriptorImageInfo{ (VkSampler)sampler->Handle, (VkImageView)houseTexture->ViewHandle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
    descriptorSet->Init(descriptorPool->vkHandle(), setLayout->vkHandle());
    houseTextureReady = true;
}

void ArcballScene::createLayouts() {
    constexpr static VkDescriptorSetLayoutBinding unique_bindings[2]{
        VkDescriptorSetLayoutBinding{
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr
        },
        VkDescriptorSetLayoutBinding{
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        }
    };
    setLayout = std::make_unique<vpr::DescriptorSetLayout>(vprObjects.device->vkHandle());
    setLayout->AddDescriptorBindings(2, unique_bindings);
    pipelineLayout = std::make_unique<vpr::PipelineLayout>(vprObjects.device->vkHandle());
    pipelineLayout->Create(&setLayout->vkHandle(), 1);
}

void ArcballScene::createShaders() {
    vert = std::make_unique<vpr::ShaderModule>(vprObjects.device->vkHandle(), "House.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    frag = std::make_unique<vpr::ShaderModule>(vprObjects.device->vkHandle(), "House.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void ArcballScene::createRenderpass() {
    renderPass = CreateBasicRenderpass(vprObjects.device, vprObjects.swapchain, depthStencil->Format);
}

void ArcballScene::createPipeline() {

    const VkPipelineShaderStageCreateInfo stages[2]{
        vert->PipelineInfo(),
        frag->PipelineInfo()
    };

    constexpr static VkVertexInputBindingDescription vertex_bindings[1]{
        VkVertexInputBindingDescription{ 0, sizeof(LoadedObjModel::vertex_t), VK_VERTEX_INPUT_RATE_VERTEX },
    };

    constexpr static VkVertexInputAttributeDescription vertex_attrs[2]{
        VkVertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
        VkVertexInputAttributeDescription{ 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec3) }
    };

    constexpr static VkPipelineVertexInputStateCreateInfo vertex_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        vertex_bindings,
        2,
        vertex_attrs
    };

    pipelineCache = std::make_unique<vpr::PipelineCache>(vprObjects.device->vkHandle(), vprObjects.physicalDevice->vkHandle(), typeid(ArcballScene).hash_code());
    pipeline = CreateBasicPipeline(vprObjects.device, 2, stages, &vertex_info, pipelineLayout->vkHandle(), renderPass, VK_COMPARE_OP_LESS_OR_EQUAL, pipelineCache->vkHandle());
}

void ArcballScene::createHouseUBO() {
    const VkBufferCreateInfo buffer_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        sizeof(ubo_data_t),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };
    houseUBO = resourceApi->CreateBuffer(&buffer_info, nullptr, 0, nullptr, uint32_t(memory_type::HOST_VISIBLE_AND_COHERENT), nullptr);
}

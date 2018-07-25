#include "TetherSimScene.hpp"
#include "TetheredSpacecraft.hpp"
#include "vpr/LogicalDevice.hpp"
#include "vpr/Fence.hpp"
#include "vpr/Renderpass.hpp"
#include "vpr/CommandPool.hpp"
#include "vpr/Framebuffer.hpp"
#include "ImGuiWrapper.hpp"
#include "vpr/Swapchain.hpp"
#include "vpr/CreateInfoBase.hpp"
#include "vpr/vkAssert.hpp"
#include "vpr/Semaphore.hpp"
#include "vpr/ShaderModule.hpp"
#include "vpr/DescriptorPool.hpp"
#include "vpr/DescriptorSet.hpp"
#include "vpr/DescriptorSetLayout.hpp"
#include "vpr/PipelineLayout.hpp"
#include "renderer_context/include/RendererContextAPI.hpp"
#include "renderer_context/include/core/RendererContext.hpp"
#include "resource_context/include/ResourceContextAPI.hpp"
#include "resource_context/include/ResourceTypes.hpp"
#include "glm/vec3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "csvFile.hpp"
#include <unordered_map>
#include <string>
#include <fstream>
#include <optional>
#include <experimental/filesystem>
#include "Arcball.hpp"

constexpr bool recompile_shaders = false;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

static const std::vector<Vertex> base_vertices{
    { { 0.5f, 0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
    { {-0.5f, 0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
    { { 0.0f,-0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
};

struct ubo_data_t {
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view;
    glm::mat4 proj;
};

struct TriangleVisualization {
    TriangleVisualization(const TriangleVisualization&) = delete;
    TriangleVisualization& operator=(const TriangleVisualization&) = delete;
    TriangleVisualization(RequiredVprObjects* objects, const StartingData* data, VkRenderPass renderpass);
    ~TriangleVisualization();
    void createResources();
    void createShaders();
    void createDescriptorObjects();
    void createPipeline(VkRenderPass pass);
    void UpdateUBO();
    void Draw(VkCommandBuffer cmd);
    RequiredVprObjects* vprObjects;
    ResourceContext_API* resourceAPI;
    VulkanResource* vbo;
    VulkanResource* ubo;
    ubo_data_t uboData;
    std::unique_ptr<vpr::ShaderModule> vert;
    std::unique_ptr<vpr::ShaderModule> frag;
    std::unique_ptr<vpr::DescriptorSetLayout> setLayout;
    std::unique_ptr<vpr::PipelineLayout> layout;
    std::unique_ptr<vpr::DescriptorSet> set;
    std::unique_ptr<vpr::DescriptorPool> pool;
    std::unique_ptr<vpr::GraphicsPipeline> pipeline;
};

TriangleVisualization::TriangleVisualization(RequiredVprObjects * objects, const StartingData * data, VkRenderPass renderpass) : vprObjects(objects), resourceAPI(data->resourceContext) {
    uboData.proj = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 1000.0f);
    uboData.proj[1][1] *= -1.0f;
    auto& cam = ArcballCamera::GetCamera();
    uboData.view = glm::lookAt(cam.Eye, cam.Target, cam.Up);
    createResources();
    createShaders();
    createDescriptorObjects();
    createPipeline(renderpass);
}

TriangleVisualization::~TriangleVisualization() {
    resourceAPI->DestroyResource(vbo);
    resourceAPI->DestroyResource(ubo);
    vert.reset();
    frag.reset();
    setLayout.reset();
    layout.reset();
    set.reset();
    pool.reset();
    pipeline.reset();
}

void TriangleVisualization::createResources() {
    const VkBufferCreateInfo vbo_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        sizeof(Vertex) * base_vertices.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    const gpu_resource_data_t data{
        base_vertices.data(),
        sizeof(Vertex) * base_vertices.size(),
        0,
        0,
        0
    };

    vbo = resourceAPI->CreateBuffer(&vbo_info, nullptr, 1, &data, uint32_t(memory_type::DEVICE_LOCAL), nullptr);

    const VkBufferCreateInfo ubo_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        sizeof(ubo_data_t),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    ubo = resourceAPI->CreateBuffer(&ubo_info, nullptr, 0, nullptr, uint32_t(memory_type::HOST_VISIBLE_AND_COHERENT), nullptr);

}

void TriangleVisualization::createShaders() {
    vert = std::make_unique<vpr::ShaderModule>(vprObjects->device->vkHandle(), "Triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    frag = std::make_unique<vpr::ShaderModule>(vprObjects->device->vkHandle(), "Triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void TriangleVisualization::createDescriptorObjects() {
    pool = std::make_unique<vpr::DescriptorPool>(vprObjects->device->vkHandle(), 1);
    pool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    pool->Create();
    setLayout = std::make_unique<vpr::DescriptorSetLayout>(vprObjects->device->vkHandle());
    setLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    layout = std::make_unique<vpr::PipelineLayout>(vprObjects->device->vkHandle());
    layout->Create(&setLayout->vkHandle(), 1);
    set = std::make_unique<vpr::DescriptorSet>(vprObjects->device->vkHandle());
    set->AddDescriptorInfo(VkDescriptorBufferInfo{ (VkBuffer)ubo->Handle, 0, sizeof(ubo_data_t) }, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    set->Init(pool->vkHandle(), setLayout->vkHandle());
}

void TriangleVisualization::createPipeline(VkRenderPass pass) {
    constexpr static VkDynamicState dynamic_states[2]{
        VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT
    };

    constexpr static VkPipelineDynamicStateCreateInfo dynamic_state_info{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        nullptr,
        0,
        2,
        dynamic_states
    };

    constexpr static VkVertexInputBindingDescription binding{
        0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX
    };

    constexpr static VkVertexInputAttributeDescription attrs[2]{
        VkVertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
        VkVertexInputAttributeDescription{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 }
    };

    vpr::GraphicsPipelineInfo info;
    info.DynamicStateInfo = dynamic_state_info;
    info.VertexInfo.vertexBindingDescriptionCount = 1;
    info.VertexInfo.pVertexBindingDescriptions = &binding;
    info.VertexInfo.vertexAttributeDescriptionCount = 2;
    info.VertexInfo.pVertexAttributeDescriptions = attrs;

    info.MultisampleInfo.sampleShadingEnable = VK_TRUE;
    info.MultisampleInfo.rasterizationSamples = TetherSimScene::SAMPLE_COUNT;

    VkGraphicsPipelineCreateInfo create_info = info.GetPipelineCreateInfo();
    create_info.layout = layout->vkHandle();
    create_info.renderPass = pass;

    const VkPipelineShaderStageCreateInfo stages[2]{
        vert->PipelineInfo(),
        frag->PipelineInfo()
    };

    create_info.stageCount = 2;
    create_info.pStages = stages;
    create_info.subpass = 0;
    create_info.basePipelineIndex = -1;
    create_info.basePipelineHandle = VK_NULL_HANDLE;

    pipeline = std::make_unique<vpr::GraphicsPipeline>(vprObjects->device->vkHandle());
    pipeline->Init(create_info, VK_NULL_HANDLE);

}

void TriangleVisualization::UpdateUBO() {
    auto& camera = ArcballCamera::GetCamera();
    uboData.view = camera.ViewMatrix;
    const gpu_resource_data_t data{
        &uboData,
        sizeof(ubo_data_t),
        0,
        0,
        0
    };
    resourceAPI->SetBufferData(ubo, 1, &data);
}

void TriangleVisualization::Draw(VkCommandBuffer cmd) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkHandle());
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout->vkHandle(), 0, 1, &set->vkHandle(), 0, nullptr);
    constexpr static VkDeviceSize offsets[1]{ 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, (VkBuffer*)&vbo->Handle, offsets);
    vkCmdDraw(cmd, static_cast<uint32_t>(base_vertices.size()), 1, 0, 0);
}

TetherSimScene::TetherSimScene() : VulkanScene() {
}

TetherSimScene::~TetherSimScene() {
    Destroy();
}

TetherSimScene & TetherSimScene::GetScene() {
    static TetherSimScene scene;
    return scene;
}

void TetherSimScene::Construct(RequiredVprObjects objects, void * user_data) {
    vprObjects = objects;
    const StartingData* starting_data = reinterpret_cast<StartingData*>(user_data);
    resourceContext = starting_data->resourceContext;
    rendererContext = starting_data->rendererContext;
    dataFilePath = starting_data->dataFilePath;
    auto& gui_ref = ImGuiWrapper::GetImGuiWrapper();
    gui = &gui_ref;
    createSemaphores();
    createCommandPools();
    createDepthStencil();
    createRenderpass();
    createFramebuffers();
    createFences();
    gui->Construct(rendererContext, resourceContext, renderpass);
    triangle = new TriangleVisualization(&objects, starting_data, renderpass);
}

void TetherSimScene::Destroy() {
    const vpr::Device* device = rendererContext->GetContext()->LogicalDevice;
    resourceContext->DestroyResource(depthStencil);
    resourceContext->DestroyResource(msaaColor);
    vkDestroyRenderPass(device->vkHandle(), renderpass, nullptr);
    framebuffers.clear(); framebuffers.shrink_to_fit();
    fences.clear(); fences.shrink_to_fit();
    graphicsPool.reset();
    computePool.reset();
    //tether.reset();
    delete triangle;
    imageAcquireSemaphore.reset();
    renderCompleteSemaphore.reset();
}

void TetherSimScene::update() {
}

void TetherSimScene::recordCommands() {

    static std::array<bool, 5> first_frame{ false, false, false, false, false };

    if (first_frame[currentBuffer]) {
        graphicsPool->ResetCmdBuffer(currentBuffer);
    }

    const vpr::Swapchain* swapchain = vprObjects.swapchain;
    const VkRect2D render_area{
        VkOffset2D{ 0, 0 },
        VkExtent2D{ vprObjects.swapchain->Extent() }
    };

    constexpr static std::array<VkClearValue, 3> clear_values{
        VkClearValue{ VkClearColorValue{ 0.0f, 0.0f, 0.5f, 1.0f } },
        VkClearValue{ VkClearColorValue{ 0.0f, 0.0f, 0.5f, 1.0f } },
        VkClearValue{ 1.0f, 0 }
    };

    const VkRect2D scissor{
        render_area
    }; 
    
    const VkViewport viewport{
        0.0f,
        0.0f,
        static_cast<float>(vprObjects.swapchain->Extent().width),
        static_cast<float>(vprObjects.swapchain->Extent().height),
        0.0f,
        1.0f
    };


    VkRenderPassBeginInfo rp_begin_info{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        renderpass,
        VK_NULL_HANDLE,
        render_area,
        static_cast<uint32_t>(clear_values.size()),
        clear_values.data()
    };

    auto& camera = ArcballCamera::GetCamera();
    camera.Update();
    triangle->UpdateUBO();

    VkCommandBuffer cmd = graphicsPool->GetCmdBuffer(currentBuffer);
    VkCommandBufferBeginInfo begin_info = vpr::vk_command_buffer_begin_info_base;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vkBeginCommandBuffer(cmd, &begin_info);
    rp_begin_info.framebuffer = framebuffers[currentBuffer]->vkHandle();
    vkCmdBeginRenderPass(cmd, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    gui->DrawFrame(CurrentFrameIdx(), cmd);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    triangle->Draw(cmd);
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    first_frame[currentBuffer] = true;
}

void TetherSimScene::draw() {
    constexpr static VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit_info{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        1,
        &imageAcquireSemaphore->vkHandle(),
        &wait_mask,
        1,
        &graphicsPool->GetCmdBuffer(currentBuffer),
        1,
        &renderCompleteSemaphore->vkHandle()
    };

    VkResult result = vkQueueSubmit(vprObjects.device->GraphicsQueue(), 1, &submit_info, fences[currentBuffer]->vkHandle());
    VkAssert(result);
}

void TetherSimScene::endFrame() {
    VkResult result = vkWaitForFences(vprObjects.device->vkHandle(), 1, &fences[currentBuffer]->vkHandle(), VK_TRUE, UINT64_MAX);
    VkAssert(result);
    result = vkResetFences(vprObjects.device->vkHandle(), 1, &fences[currentBuffer]->vkHandle());
    VkAssert(result);
    //tether->Increment();
}

void TetherSimScene::createCommandPools() {
    VkCommandPoolCreateInfo create_info = vpr::vk_command_pool_info_base;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    create_info.queueFamilyIndex = vprObjects.device->QueueFamilyIndices.Graphics;
    graphicsPool = std::make_unique<vpr::CommandPool>(vprObjects.device->vkHandle(), create_info);
    graphicsPool->AllocateCmdBuffers(vprObjects.swapchain->ImageCount(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    create_info.queueFamilyIndex = vprObjects.device->QueueFamilyIndices.Compute;
    computePool = std::make_unique<vpr::CommandPool>(vprObjects.device->vkHandle(), create_info);
    computePool->AllocateCmdBuffers(vprObjects.swapchain->ImageCount(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void TetherSimScene::createDepthStencil() {

    const vpr::Swapchain* swapchain = rendererContext->GetContext()->Swapchain;
    const vpr::Device* device = rendererContext->GetContext()->LogicalDevice;

    const VkImageCreateInfo image_info{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        device->FindDepthFormat(),
        VkExtent3D{ swapchain->Extent().width, swapchain->Extent().height, 1 },
        1,
        1,
        SAMPLE_COUNT,
        device->GetFormatTiling(device->FindDepthFormat(), VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
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
        device->FindDepthFormat(),
        {},
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }
    };

    depthStencil = resourceContext->CreateImage(&image_info, &view_info, 0, nullptr, uint32_t(memory_type::DEVICE_LOCAL), nullptr);

}

void TetherSimScene::createMultisampledAttachment() {

    const vpr::Swapchain* swapchain = rendererContext->GetContext()->Swapchain;
    const vpr::Device* device = rendererContext->GetContext()->LogicalDevice;

    const VkImageCreateInfo color_image_info{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        swapchain->ColorFormat(),
        VkExtent3D{ swapchain->Extent().width, swapchain->Extent().height, 1 },
        1,
        1,
        SAMPLE_COUNT,
        device->GetFormatTiling(swapchain->ColorFormat(), VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT),
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    const VkImageViewCreateInfo color_image_view_info{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        VK_NULL_HANDLE,
        VK_IMAGE_VIEW_TYPE_2D,
        swapchain->ColorFormat(),
        VkComponentMapping{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    };
    
    msaaColor = resourceContext->CreateImage(&color_image_info, &color_image_view_info, 0, nullptr, uint32_t(memory_type::DEVICE_LOCAL), nullptr);

}

void TetherSimScene::createAttachmentDescriptions() {
    attachmentDescriptions.fill(vpr::vk_attachment_description_base);

    const vpr::Swapchain* swapchain = rendererContext->GetContext()->Swapchain;
    const vpr::Device* device = rendererContext->GetContext()->LogicalDevice;

    attachmentDescriptions[0].format = swapchain->ColorFormat();
    attachmentDescriptions[0].samples = SAMPLE_COUNT;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachmentDescriptions[1].format = swapchain->ColorFormat();
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachmentDescriptions[2].format = device->FindDepthFormat();
    attachmentDescriptions[2].samples = SAMPLE_COUNT;
    attachmentDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

void TetherSimScene::createAttachmentReferences() {
    attachmentReferences[0] = VkAttachmentReference{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    attachmentReferences[1] = VkAttachmentReference{ 1, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
    attachmentReferences[2] = VkAttachmentReference{ 2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
}


void TetherSimScene::createRenderpass() {

    createMultisampledAttachment();
    createAttachmentDescriptions();
    createAttachmentReferences();
    subpassDescription = vpr::vk_subpass_description_base;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReferences[0];
    subpassDescription.pResolveAttachments = &attachmentReferences[1];
    subpassDescription.pDepthStencilAttachment = &attachmentReferences[2];
    
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo create_info = vpr::vk_render_pass_create_info_base;
    create_info.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    create_info.pAttachments = attachmentDescriptions.data();
    create_info.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    create_info.pDependencies = subpassDependencies.data();
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpassDescription;

    const vpr::Device* device = rendererContext->GetContext()->LogicalDevice;
    VkResult result = vkCreateRenderPass(device->vkHandle(), &create_info, nullptr, &renderpass);
    VkAssert(result);

}

void TetherSimScene::createFramebuffers() {
    const vpr::Swapchain* swapchain = rendererContext->GetContext()->Swapchain;
    const vpr::Device* device = rendererContext->GetContext()->LogicalDevice;
    std::array<VkImageView, 3> image_views{ (VkImageView)msaaColor->ViewHandle, VK_NULL_HANDLE, (VkImageView)depthStencil->ViewHandle };
    VkFramebufferCreateInfo create_info = vpr::vk_framebuffer_create_info_base;
    create_info.attachmentCount = static_cast<uint32_t>(image_views.size());
    create_info.pAttachments = image_views.data();
    create_info.renderPass = renderpass;
    create_info.width = swapchain->Extent().width;
    create_info.height = swapchain->Extent().height;
    create_info.layers = 1;

    for (size_t i = 0; i < size_t(swapchain->ImageCount()); ++i) {
        image_views[1] = swapchain->ImageView(i);
        framebuffers.emplace_back(std::make_unique<vpr::Framebuffer>(device->vkHandle(), create_info));
    }

}

void TetherSimScene::createFences() {
    const vpr::Device* device = rendererContext->GetContext()->LogicalDevice;
    for (size_t i = 0; i < framebuffers.size(); ++i) {
        fences.emplace_back(std::make_unique<vpr::Fence>(device->vkHandle(), 0));
    }
}

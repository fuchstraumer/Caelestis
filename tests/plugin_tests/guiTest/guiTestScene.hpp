#pragma once
#ifndef TETHER_VISUALIZATION_HPP
#define TETHER_VISUALIZATION_HPP
#include "VulkanScene.hpp"
#include <string>
#include <vulkan/vulkan.h>
#include <vector>
#include <array>

class TetheredSpacecraft;
class ImGuiWrapper;
struct RendererContext_API;
struct ResourceContext_API;
struct VulkanResource;
struct TriangleVisualization;

namespace vpr {
    class CommandPool;
    class Framebuffer;
    class Fence;
}

struct StartingData {
    RendererContext_API* rendererContext;
    ResourceContext_API* resourceContext;
    std::string dataFilePath;
};

class TetherSimScene : public VulkanScene {
    TetherSimScene();
    ~TetherSimScene();
public:

    static TetherSimScene& GetScene();
    void Construct(RequiredVprObjects objects, void* user_data) final;
    void Destroy() final;

    inline static VkSampleCountFlagBits SAMPLE_COUNT = VK_SAMPLE_COUNT_4_BIT;

protected:

    void update() final;
    void recordCommands() final;
    void draw() final;
    void endFrame() final;

    void createCommandPools();
    void createDepthStencil();
    void createMultisampledAttachment();
    void createAttachmentDescriptions();
    void createAttachmentReferences();
    void createRenderpass();
    void createFramebuffers();
    void createFences();

    std::string dataFilePath;
    VulkanResource* msaaColor;
    VulkanResource* depthStencil;
    RendererContext_API* rendererContext;
    ResourceContext_API* resourceContext;
    std::array<VkAttachmentDescription, 3> attachmentDescriptions;
    std::array<VkAttachmentReference, 3> attachmentReferences;
    VkSubpassDescription subpassDescription;
    std::array<VkSubpassDependency, 2> subpassDependencies;
    VkRenderPass renderpass;
    std::vector<std::unique_ptr<vpr::Fence>> fences;
    std::unique_ptr<vpr::CommandPool> graphicsPool;
    std::unique_ptr<vpr::CommandPool> computePool;
    std::vector<std::unique_ptr<vpr::Framebuffer>> framebuffers;
    //std::unique_ptr<TetheredSpacecraft> tether;
    ImGuiWrapper* gui;
    TriangleVisualization* triangle;

};

#endif //!TETHER_VISUALIZATION_HPP
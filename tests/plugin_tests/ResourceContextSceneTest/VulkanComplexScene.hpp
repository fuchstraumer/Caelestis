#pragma once
#ifndef VULKAN_COMPLEX_SCENE_HPP
#define VULKAN_COMPLEX_SCENE_HPP
#include "VulkanScene.hpp"
#include "ForwardDecl.hpp"
#include "CommonCreationFunctions.hpp"
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <atomic>

namespace vpr {
    class PipelineCache;
}

struct ResourceContext_API;
struct VulkanResource;

class VulkanComplexScene : public VulkanScene {
    VulkanComplexScene();
    ~VulkanComplexScene();
public:

    static VulkanComplexScene& GetScene();

    void Construct(RequiredVprObjects objects, void* user_data) final;
    void Destroy() final;

    static void* LoadObjFile(const char* fname);
    static void* LoadJpegImage(const char* fname);
    static void* LoadCompressedTexture(const char* fname);

    void CreateHouseMesh(void* obj_data);
    void CreateHouseTexture(void* texture_data);
    void CreateSkyboxTexture(void* texture_data);

    void WaitForAllLoaded();

    struct ubo_data_t {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

protected:

    void update() final;
    void recordCommands() final;
    void draw() final;
    void endFrame() final;

    const ResourceContext_API* resourceContext;
    VulkanResource* sampler;
    VulkanResource* houseVBO;
    VulkanResource* houseEBO;
    VulkanResource* houseTexture;
    VulkanResource* skyboxVBO;
    VulkanResource* skyboxTexture;
    VulkanResource* sharedUBO;

    DepthStencil depthStencil;
    std::unique_ptr<vpr::ShaderModule> houseVert, houseFrag;
    std::unique_ptr<vpr::ShaderModule> skyboxVert, skyboxFrag;
    std::unique_ptr<vpr::PipelineLayout> housePipelineLayout, skyboxPipelineLayout;
    std::unique_ptr<vpr::DescriptorPool> descriptorPool;
    std::unique_ptr<vpr::DescriptorSetLayout> houseSetLayout, skyboxSetLayout;
    std::unique_ptr<vpr::DescriptorSet> houseSet, skyboxSet, samplerSet;
    std::unique_ptr<vpr::PipelineCache> pipelineCacheMaster;
    VkPipelineCache houseCache, skyboxCache;
    VkPipeline housePipeline, skyboxPipeline;
    VkRenderPass renderPass;
    std::vector<VkFence> fences;
    std::vector<VkFramebuffer> framebuffers;

    std::atomic<bool> houseTextureReady;
    std::atomic<bool> houseMeshReady;
    std::atomic<bool> skyboxTextureReady;

};


#endif //!VULKAN_COMPLEX_SCENE_HPP

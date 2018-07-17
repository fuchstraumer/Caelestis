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
    static void DestroyObjFileData(void* obj_file);
    static void* LoadJpegImage(const char* fname);
    static void DestroyJpegFileData(void* jpeg_file);
    static void* LoadCompressedTexture(const char* fname);
    static void DestroyCompressedTextureData(void* compressed_texture);

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

    void createSampler();
    void createUBO();

    void createDescriptorPool();
    void createDescriptorSetLayouts();
    void createBaseDescriptorSet();
    void createPipelineLayouts();
    void createShaders();
    void createFramebuffers();
    void createRenderpass();
    void createHousePipeline();
    void createSkyboxPipeline();

    // has to wait for loading to complete
    void createTextureDescriptorSets();

    const ResourceContext_API* resourceContext;
    VulkanResource* sampler;
    VulkanResource* houseVBO0;
    VulkanResource* houseVBO1;
    VulkanResource* houseEBO;
    VulkanResource* houseTexture;
    VulkanResource* skyboxVBO;
    VulkanResource* skyboxTexture;
    VulkanResource* sharedUBO;

    DepthStencil depthStencil;
    std::unique_ptr<vpr::ShaderModule> houseVert, houseFrag;
    std::unique_ptr<vpr::ShaderModule> skyboxVert, skyboxFrag;
    std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
    std::unique_ptr<vpr::DescriptorPool> descriptorPool;
    std::unique_ptr<vpr::DescriptorSetLayout> baseSetLayout, textureSetLayout;
    std::unique_ptr<vpr::DescriptorSet> houseSet, skyboxSet, baseSet;
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

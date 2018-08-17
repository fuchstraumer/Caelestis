#pragma once
#ifndef ARCBALL_CAMERA_TEST_SCENE_HPP
#define ARCBALL_CAMERA_TEST_SCENE_HPP
#include "VulkanScene.hpp"
#include <vulkan/vulkan.h>
#include "glm/mat4x4.hpp"
#include "ForwardDecl.hpp"
#include <memory>
#include <vector>
#include <atomic>

struct VulkanResource;

struct RequiredInitialData {
    struct RendererContext_API* rendererAPI;
    struct ResourceContext_API* resourceAPI;
    class Camera* camera;
    class ImGuiWrapper* gui;
};

class ArcballScene : public VulkanScene {
    ArcballScene();
    ~ArcballScene();
public:

    static ArcballScene& GetScene();

    void Construct(RequiredVprObjects objects, void* user_data) final;
    void Destroy() final;

    static void* LoadObj(const char* fname, void* user_data);
    static void DestroyObj(void* obj_file);
    static void* LoadPNG(const char* fname, void* user_data);
    static void DestroyPNG(void* jpeg_data);

    void CreateHouseMesh(void* obj_data);
    void CreateHouseTexture(void* texture_data);

    void UpdateHouseModelMatrix(glm::mat4 model_matrix);

    struct ubo_data_t {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

protected:

    void update() final;
    void recordCommands() final;
    void renderHouse(VkCommandBuffer cmd);
    void draw() final;
    void endFrame() final;

    void createCmdPool();
    void createSampler();
    void createFences();
    void destroyFences();
    void createDepthStencil();
    void createFramebuffers();
    void destroyFramebuffers();
    void createDescriptors();
    void updateDescriptorSets();
    void createLayouts();
    void createShaders();
    void createRenderpass();
    void createPipeline();
    void createHouseUBO();

    class Camera* camera;
    class ImGuiWrapper* gui;
    ubo_data_t uboData;
    const struct ResourceContext_API* resourceApi;
    VulkanResource* sampler;
    VulkanResource* houseVBO;
    VulkanResource* houseEBO;
    VulkanResource* houseTexture;
    VulkanResource* houseUBO;
    std::unique_ptr<vpr::CommandPool> cmdPool;
    std::unique_ptr<vpr::ShaderModule> vert, frag;
    std::unique_ptr<vpr::DescriptorPool> descriptorPool; 
    std::unique_ptr<vpr::DescriptorSet> descriptorSet;
    std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
    std::unique_ptr<vpr::DescriptorSetLayout> setLayout;
    std::unique_ptr<vpr::PipelineCache> pipelineCache;
    struct DepthStencil* depthStencil;
    VkPipeline pipeline;
    VkRenderPass renderPass;

    std::vector<std::unique_ptr<vpr::Fence>> fences;
    std::vector<std::unique_ptr<vpr::Framebuffer>> framebuffers;

    uint32_t houseIndexCount;
    std::atomic<bool> houseMeshReady = false;
    std::atomic<bool> houseTextureReady = false;

};

#endif //!ARCBALL_CAMERA_TEST_SCENE_HPP

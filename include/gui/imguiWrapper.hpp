#pragma once
#ifndef VULPES_VK_IMGUI_WRAPPER_H
#define VULPES_VK_IMGUI_WRAPPER_H
#include "vpr_stdafx.h"
#include "../../imgui/imgui.h"
#include "../command/TransferPool.hpp"
#include "../resource/Texture.hpp"
#include "../resource/Buffer.hpp"
#include "../render/GraphicsPipeline.hpp"
#include "../resource/PipelineCache.hpp"
#include "../resource/ShaderModule.hpp"

namespace vpr {

    struct imguiSettings {
        bool displayMeshes = true;
        bool displaySkybox = true;
        std::array<float, 200> frameTimes;
        float frameTimeMin = 9999.0f, frameTimeMax = 0.0f;
    };

    class imguiWrapper  {
        imguiWrapper(const imguiWrapper&) = delete;
        imguiWrapper& operator=(const imguiWrapper&) = delete;
    public:

        imguiWrapper() = default;
        ~imguiWrapper();

        void Init(const Device* dvc, const VkRenderPass& renderpass);

        void UploadTextureData(TransferPool* transfer_pool);

        void NewFrame(Instance* instance);

        void UpdateBuffers();

        void updateMouseActions(Instance* instance);

        void DrawFrame(VkCommandBuffer& cmd);

        imguiSettings settings;

        int imgWidth, imgHeight;

    private:

        void createResources();
        void createDescriptorPools();
        size_t loadFontTextureData();
        void uploadFontTextureData(const size_t& font_texture_size);
        void createFontTexture();
        void createDescriptorLayout();
        void createPipelineLayout();
        void allocateDescriptors();
        void updateDescriptors();
        void setupGraphicsPipelineInfo();
        void setupGraphicsPipelineCreateInfo(const VkRenderPass& renderpass);
        void updateImguiSpecialKeys() noexcept;
        void validateBuffers();
        void updateBufferData();
        void updateFramegraph(const float& frame_time);
        void freeMouse(Instance* instance);
        void captureMouse(Instance* instance);

        static float mouseWheel;
        std::array<bool, 3> mouseClick;
        size_t frameIdx;
        VkCommandBuffer graphicsCmd;
        const Device* device;

        unsigned char* fontTextureData;
        std::shared_ptr<PipelineCache> cache;
        std::unique_ptr<GraphicsPipeline> pipeline;
        std::unique_ptr<Buffer> vbo, ebo;
        std::unique_ptr<Texture<gli::texture2d>> texture;
        std::unique_ptr<ShaderModule> vert, frag;

        GraphicsPipelineInfo pipelineStateInfo;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo;

        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;
        VkDescriptorSetLayout descriptorSetLayout;
        VkPipelineLayout pipelineLayout;

        VkBuffer textureStaging;
        VkBufferImageCopy stagingToTextureCopy;
        VkImage fontImage;
        VkImageView fontView;
        VkSampler fontSampler;

        VkDescriptorImageInfo fontInfo;
        VkWriteDescriptorSet fontWriteSet;

    };

}
#endif // !VULPES_VK_IMGUI_WRAPPER_H

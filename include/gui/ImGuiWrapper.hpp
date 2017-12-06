#pragma once
#ifndef VULPES_VK_IMGUI_WRAPPER_H
#define VULPES_VK_IMGUI_WRAPPER_H
#include "vpr_stdafx.h"
#include "imgui/imgui.h"
#include "ForwardDecl.hpp"
#include "resource/Texture.hpp"
#include "render/GraphicsPipeline.hpp"

namespace vpsk {

    struct imguiSettings {
        bool displayMeshes = true;
        bool displaySkybox = true;
        std::array<float, 200> frameTimes;
        float frameTimeMin = 9999.0f, frameTimeMax = 0.0f;
    };

    class ImGuiWrapper {
        ImGuiWrapper(const ImGuiWrapper&) = delete;
        ImGuiWrapper& operator=(const ImGuiWrapper&) = delete;
    public:

        ImGuiWrapper() = default;
        ~ImGuiWrapper();

        void Init(const vpr::Device* dvc, const VkRenderPass& renderpass, vpr::DescriptorPool* descriptor_pool);
        void UploadTextureData(vpr::TransferPool* transfer_pool);
        void NewFrame(vpr::Instance* instance);
        void UpdateBuffers();
        void updateMouseActions(vpr::Instance* instance);
        void DrawFrame(VkCommandBuffer& cmd);
        imguiSettings settings;
        int imgWidth, imgHeight;

    private:

        void createResources();
        size_t loadFontTextureData();
        void uploadFontTextureData(const size_t& font_texture_size);
        void createFontTexture();
        void createDescriptorLayout(vpr::DescriptorPool* descriptor_pool);
        void createPipelineLayout();
        void setupGraphicsPipelineInfo();
        void setupGraphicsPipelineCreateInfo(const VkRenderPass& renderpass);
        void createGraphicsPipeline();
        void updateImguiSpecialKeys() noexcept;
        void validateBuffers();
        void updateBufferData();
        void updateFramegraph(const float& frame_time);
        void freeMouse(vpr::Instance* instance);
        void captureMouse(vpr::Instance* instance);

        static float mouseWheel;
        std::array<bool, 3> mouseClick;
        size_t frameIdx;
        VkCommandBuffer graphicsCmd;
        const vpr::Device* device;

        std::shared_ptr<vpr::PipelineCache> cache;
        std::unique_ptr<vpr::GraphicsPipeline> pipeline;
        std::unique_ptr<vpr::Buffer> vbo, ebo;
        std::unique_ptr<vpr::Texture<gli::texture2d>> texture;
        std::unique_ptr<vpr::ShaderModule> vert, frag;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> font;
        std::unique_ptr<vpr::PipelineLayout> layout;
        std::unique_ptr<vpr::DescriptorSet> descriptorSet;

        vpr::GraphicsPipelineInfo pipelineStateInfo;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo;

        unsigned char* fontTextureData;
        VkBuffer textureStaging;
        VkBufferImageCopy stagingToTextureCopy;

        VkDescriptorImageInfo fontInfo;
        VkWriteDescriptorSet fontWriteSet;

    };

}
#endif // !VULPES_VK_IMGUI_WRAPPER_H

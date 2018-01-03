#include "ForwardDecl.hpp"
#include "scene/BaseScene.hpp"
#include "core/Instance.hpp"
#include "render/GraphicsPipeline.hpp"
#include "render/Renderpass.hpp"
#include "render/Swapchain.hpp"
#include "command/CommandPool.hpp"
#include "command/TransferPool.hpp"
#include "resource/Buffer.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/Texture.hpp"
#include "resource/Allocator.hpp"
#include "resource/PipelineCache.hpp"
#include "resource/DescriptorPool.hpp"
#include "geometries/Skybox.hpp"
#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP
#include "../tinyobjloader/tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "glm/gtx/hash.hpp"
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <experimental/filesystem>
#include <vector>

namespace fs = std::experimental::filesystem;

class TetherScene : public vpsk::BaseScene {
public:
    TetherScene(const std::vector<fs::path>& data_file_paths);
    ~TetherScene();

    void WindowResized() final;
    void RecreateObjects() final;
    void RecordCommands() final;

private:

    void endFrame(const size_t& idx) final;

    struct instance_data_t {
        glm::vec3 Offset;
    };

    struct vertex_shader_ubo_t {
        glm::mat4 view, projection;
    };

    struct timestep_offsets_t {
        timestep_offsets_t(const size_t& num_elements, const size_t& num_steps) {
            offsets.resize(num_elements * num_steps);
            curr = offsets.cbegin();
        }
        
        void SetData(const std::vector<fs::path>& files);

        void Step() {
            curr += num_elements;
        }

        const void* CurrentAddress() const noexcept {
            return &(*curr);
        }

        
        std::vector<glm::vec3> offsets;
        std::vector<glm::vec3>::const_iterator curr;
        size_t num_elements;
    } offsetData;

    void createVBO();
    void createIBO();
    void uploadBuffers();
    void createShaders();
    void createPipelineLayout();
    void createPipelineCache();
    void setPipelineState();
    void setVertexData();
    void createPipeline();

    std::unique_ptr<vpr::Buffer> vbo;
    std::unique_ptr<vpr::Buffer> ibo;
    std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
    std::unique_ptr<vpr::GraphicsPipeline> pipeline;
    std::unique_ptr<vpr::PipelineCache> cache;
    std::unique_ptr<vpr::ShaderModule> vert, frag;
    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    vpr::GraphicsPipelineInfo pipelineStateInfo;
    VkViewport viewport;
    VkRect2D scissor;
    size_t numElements;
};


constexpr static std::array<float, 12> element_vertices {
    -0.5f,-0.5f, 0.0f,
     0.5f,-0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
     0.5f, 0.5f, 0.0f
};

void TetherScene::createVBO() {
    vbo = std::make_unique<vpr::Buffer>(device.get());
    vbo->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(float) * 12);   
}

void TetherScene::createIBO() {
    ibo = std::make_unique<vpr::Buffer>(device.get());
    ibo->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::vec3) * numElements);
}

void TetherScene::uploadBuffers() {
    const auto& cmd = transferPool->Begin();
    ibo->CopyTo(offsetData.CurrentAddress(), cmd, sizeof(glm::vec3) * 12, 0);
    vbo->CopyTo(element_vertices.data(), cmd, sizeof(float) * 12, 0);
    transferPool->Submit();
}

void TetherScene::createShaders() {
    vert = std::make_unique<vpr::ShaderModule>(device.get(), "rsrc/shaders/tether/tether.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    frag = std::make_unique<vpr::ShaderModule>(device.get(), "rsrc/shaders/tether/tether.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}
#include "ArcballHelper.hpp"
#include "resource_context/include/ResourceTypes.hpp"
#include "vpr/PipelineLayout.hpp"
#include "vpr/ShaderModule.hpp"
#include "vpr/GraphicsPipeline.hpp"
#include "vpr/LogicalDevice.hpp"
#include "vpr/PhysicalDevice.hpp"
#include "vpr/Swapchain.hpp"
#include "vpr/PipelineCache.hpp"
#include "ArcballCameraController.hpp"
#include "resource_context/include/ResourceContextAPI.hpp"
#include "renderer_context/include/RendererContextAPI.hpp"
#include "renderer_context/include/core/RendererContext.hpp"
#include "glm/gtx/norm.hpp"
#include "CommonCreationFunctions.hpp"

ArcballHelper::ArcballHelper() {}

ArcballHelper::~ArcballHelper() {
    Destroy();
}

ArcballHelper& ArcballHelper::GetArcballHelper() {
    static ArcballHelper helper;
    return helper;
}

void ArcballHelper::Construct(const RendererContext_API * renderer_api, const ResourceContext_API * resource_api, VkRenderPass renderpass) {
    rendererApi = renderer_api;
    resourceApi = resource_api;
    createPipelineLayout();
    createShaders();
    createPipelineCache();
    createPipelines(renderpass);
}

void ArcballHelper::Destroy() {
    const vpr::Device* dvc = rendererApi->GetContext()->LogicalDevice;
    vkDestroyPipeline(dvc->vkHandle(), pointsPipeline, nullptr);
    vkDestroyPipeline(dvc->vkHandle(), linesPipeline, nullptr);
    vert.reset();
    frag.reset();
    pipelineCache.reset();
    pipelineLayout.reset();
    drag.Destroy(resourceApi);
    rim.Destroy(resourceApi);
    result.Destroy(resourceApi);
    for (auto& mesh : constraintMeshes) {
        mesh.Destroy(resourceApi);
    }
}

void ArcballHelper::Render(const glm::mat4 & proj_view, VkCommandBuffer cmd) {

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, linesPipeline);
    vkCmdPushConstants(cmd, pipelineLayout->vkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &proj_view);
    
    if (showDrag && (drag.vertexCount > 0)) {
        drawHelperMesh(cmd, drag);
    }

    if (showRim && (rim.vertexCount > 0)) {
        drawHelperMesh(cmd, rim);
    }

    if (showResult && (result.vertexCount > 0)) {
        drawHelperMesh(cmd, result);
    }

    if (showConstraints) {
        if (usePointsPipeline) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pointsPipeline);
        }
        for (auto& mesh : constraintMeshes) {
            if (mesh.vertexCount == 0) {
                continue;
            }
            drawHelperMesh(cmd, mesh);
        }
    }

}

void ArcballHelper::Update(const ArcballCameraController & arcball) {
    updateDragArc(arcball);
    updateConstraintArcs(arcball);
    updateResultArc(arcball);
    updateRimArc(arcball);
}

void ArcballHelper::DragArcVisibility(bool render) {
    showDrag = render;
}

void ArcballHelper::ConstraintArcsVisibility(bool render) {
    showConstraints = render;
}

void ArcballHelper::ResultArcVisbility(bool render) {
    showResult = render;
}

void ArcballHelper::RimArcVisibility(bool render) {
    showRim = render;
}

void ArcballHelper::drawHelperMesh(VkCommandBuffer cmd, const helperMesh & mesh) const {
    constexpr static VkDeviceSize offsets[1]{ 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, (VkBuffer*)&mesh.VBO->Handle, offsets);
    vkCmdPushConstants(cmd, pipelineLayout->vkHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), sizeof(glm::vec4), &mesh.Color);
    vkCmdDraw(cmd, mesh.vertexCount, 1, 0, 0);
}

void ArcballHelper::updateDragArc(const ArcballCameraController & arcball) {
    if (arcball.dragging) {
        drag.Color = glm::vec4(1.0f, 1.0f, 0.0f, 0.5f);
        if (arcball.constraint.CurrentConstraintAxis != AxisSet::None) {
            drag.Color = axisIndexColor(arcball.constraint.Nearest);
        }

        fillArc(arcball, drag.positions, arcball.drag.From, arcball.drag.To);
        updateMesh(drag);
    }
}

void ArcballHelper::updateRimArc(const ArcballCameraController & arcball) {
    if (!overrideRim) {
        fillCircle(arcball, rim.positions);
    }
    updateMesh(rim);
}

void ArcballHelper::updateConstraintArcs(const ArcballCameraController & arcball) {
    if (!arcball.allowConstraints) {
        return;
    }

    overrideRim = false;
    usePointsPipeline = false;
    if (arcball.dragging) {
        if (arcball.constraint.CurrentConstraintAxis != AxisSet::None) {
            fillConstraint(arcball, arcball.constraint.Nearest);
            //usePointsPipeline = true;
        }
    }
    else {
        if (arcball.constraint.CurrentConstraintAxis != AxisSet::None) {
            for (size_t i = 0; i < constraintMeshes.size(); ++i) {
                fillConstraint(arcball, i);
            }
        }
    }
}

void ArcballHelper::updateResultArc(const ArcballCameraController & arcball) {
    fillArc(arcball, result.positions, arcball.result.From, arcball.result.To);
    updateMesh(result);
}

void ArcballHelper::fillConstraint(const ArcballCameraController & arcball, size_t idx) {
    auto& curr_node = constraintMeshes[idx];
    glm::vec3 axis = arcball.constraint.Available[idx];
    if (axis.z == 1.0f) {
        fillCircle(arcball, curr_node.positions);
        overrideRim = true;
    }
    else {
        fillHalfArc(arcball, curr_node.positions, axis);
    }

    updateMesh(curr_node);

}

void ArcballHelper::fillCircle(const ArcballCameraController & arcball, std::vector<glm::vec3>& positions) const {
    constexpr static size_t segments = 128;
    const float radius = arcball.radius;
    constexpr static float PI_2 = glm::pi<float>() * 2.0f;
    constexpr static float SEGMENT_SIZE = PI_2 / static_cast<float>(segments);

    for (float i = 0.0f; i < PI_2; i += SEGMENT_SIZE) {
        positions.emplace_back(cosf(i), sinf(i), 0.0f);
    }

}

void ArcballHelper::fillArc(const ArcballCameraController & arcball, std::vector<glm::vec3>& positions, glm::vec3 from, glm::vec3 to) const {
    constexpr static size_t ARC_BISECTS = 5;
    constexpr static size_t ARC_SEGMENTS = 32;

    std::array<glm::vec3, ARC_SEGMENTS + 1> points;
    points[0] = from;
    points[1] = to;
    points[ARC_SEGMENTS] = to;

    for (size_t i = 0; i < ARC_BISECTS; ++i) {
        points[1] = bisect(points[0], points[1]);
    }

    auto push = [&positions, &arcball](glm::vec3 p) {
        positions.emplace_back(p * arcball.radius);
    };

    push(points[0]);
    push(points[1]);

    const float dot_two = glm::dot(points[0], points[1]) * 2.0f;
    for (size_t i = 2; i < ARC_SEGMENTS; ++i) {
        points[i] = (points[i - 1] * dot_two) - points[i - 2];
        push(points[i]);
    }

    push(points[ARC_SEGMENTS]);

}

void ArcballHelper::fillHalfArc(const ArcballCameraController & arcball, std::vector<glm::vec3>& positions, glm::vec3 axis) const {
    glm::vec3 mirrorPoint;
    if (axis.z != 1.0f) {
        mirrorPoint = glm::vec3{
            axis.y, -axis.z, 0.0f
        };
        mirrorPoint = glm::normalize(mirrorPoint);
    }
    else {
        mirrorPoint.x = 0.0f;
        mirrorPoint.y = 1.0f;
    }

    glm::vec3 midpoint = glm::cross(mirrorPoint, axis);

    fillArc(arcball, positions, mirrorPoint, midpoint);
    fillArc(arcball, positions, midpoint, -mirrorPoint);

}

void ArcballHelper::updateMesh(helperMesh & mesh) {

    if (mesh.positions.empty()) {
        return;
    }

    const gpu_resource_data_t data{
        mesh.positions.data(),
        sizeof(glm::vec3) * mesh.positions.size(),
        0,
        0,
        0
    };

    if (VkDeviceSize(data.DataSize) > mesh.vboSize) {
        if (mesh.VBO) {
            resourceApi->DestroyResource(mesh.VBO);
        }
        // have to recreate buffer.
        mesh.vboSize = static_cast<VkDeviceSize>(data.DataSize);
        const VkBufferCreateInfo info{
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            nullptr,
            0,
            mesh.vboSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr
        };
        mesh.VBO = resourceApi->CreateBuffer(&info, nullptr, 1, &data, uint32_t(memory_type::HOST_VISIBLE_AND_COHERENT), nullptr);
    }
    else {
        resourceApi->SetBufferData(mesh.VBO, 1, &data);
    }

    mesh.vertexCount = static_cast<uint32_t>(mesh.positions.size());

    // data is uploaded instantly when using host visible, so we can clear and shrink the mesh positions buffer
    mesh.positions.clear();
    mesh.positions.shrink_to_fit();
}

glm::vec3 ArcballHelper::bisect(glm::vec3 a, glm::vec3 b) const {
    constexpr static float LOCAL_EPSILON = 1.0e-8f;
    glm::vec3 v = a + b;
    const float length2 = glm::length2(v);
    if (length2 < LOCAL_EPSILON) {
        v = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    else {
        v *= (1.0f / sqrtf(length2));
    }
    return v;
}

glm::vec4 ArcballHelper::axisIndexColor(size_t idx) const {
    switch (idx) {
    case 0:
        return glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
    case 1:
        return glm::vec4(0.0f, 1.0f, 0.0f, 0.5f);
    case 2:
        return glm::vec4(0.0f, 0.2f, 0.7f, 0.5f);
    default:
        throw std::domain_error("Invalid axis index to axisIndexColor function.");
    }
    return glm::vec4();
}

void ArcballHelper::createPipelineLayout() {
    constexpr static VkPushConstantRange ranges[2]{
        VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) },
        VkPushConstantRange{ VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), sizeof(glm::vec4) }
    };
    pipelineLayout = std::make_unique<vpr::PipelineLayout>(rendererApi->GetContext()->LogicalDevice->vkHandle());
    pipelineLayout->Create(ranges, 2);
}

void ArcballHelper::createShaders() {
    const vpr::Device* dvc = rendererApi->GetContext()->LogicalDevice;
    vert = std::make_unique<vpr::ShaderModule>(dvc->vkHandle(), "Helper.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    frag = std::make_unique<vpr::ShaderModule>(dvc->vkHandle(), "Helper.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void ArcballHelper::createPipelineCache() {
    pipelineCache = std::make_unique<vpr::PipelineCache>(rendererApi->GetContext()->LogicalDevice->vkHandle(), rendererApi->GetContext()->PhysicalDevices[0]->vkHandle(), typeid(ArcballHelper).hash_code());
}

void ArcballHelper::createPipelines(VkRenderPass pass) {
    const VkPipelineShaderStageCreateInfo stages[2]{
        vert->PipelineInfo(),
        frag->PipelineInfo()
    };

    constexpr static VkVertexInputBindingDescription bind_descr{
        0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX
    };

    constexpr static VkVertexInputAttributeDescription attr_descr{
        0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0
    };

    constexpr static VkPipelineVertexInputStateCreateInfo vertex_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &bind_descr,
        1,
        &attr_descr
    };

    linesPipeline = CreateBasicPipeline(rendererApi->GetContext()->LogicalDevice, 2, stages, &vertex_info, pipelineLayout->vkHandle(), pass, VK_COMPARE_OP_ALWAYS, pipelineCache->vkHandle(),
        VK_NULL_HANDLE, VK_CULL_MODE_NONE, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    pointsPipeline = CreateBasicPipeline(rendererApi->GetContext()->LogicalDevice, 2, stages, &vertex_info, pipelineLayout->vkHandle(), pass, VK_COMPARE_OP_ALWAYS, pipelineCache->vkHandle(),
        linesPipeline, VK_CULL_MODE_NONE, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

}

void ArcballHelper::helperMesh::Destroy(const ResourceContext_API * api) {
    if (VBO) {
        api->DestroyResource(VBO);
        VBO = nullptr;
    }
    vertexCount = 0;
    positions.clear(); positions.shrink_to_fit();
    vboSize = 0;
}

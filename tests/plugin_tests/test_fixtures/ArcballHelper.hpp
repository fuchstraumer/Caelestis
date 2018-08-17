#pragma once
#ifndef ARCBALL_HELPER_OBJECT_HPP
#define ARCBALL_HELPER_OBJECT_HPP
#include "ForwardDecl.hpp"
#include "glm/fwd.hpp"
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <memory>

class ArcballCameraController;
struct VulkanResource;

class ArcballHelper {
    ArcballHelper();
    ~ArcballHelper();
public:

    static ArcballHelper& GetArcballHelper();

    void Construct(const struct RendererContext_API* renderer_api, const struct ResourceContext_API* resource_api, VkRenderPass renderpass);
    void Destroy();

    void Render(const glm::mat4& proj_view, VkCommandBuffer cmd);

    void Update(const ArcballCameraController& arcball);
    
    void DragArcVisibility(bool render);
    void ConstraintArcsVisibility(bool render);
    void ResultArcVisbility(bool render);
    void RimArcVisibility(bool render);

private:

    struct helperMesh {
        void Destroy(const struct ResourceContext_API* api);
        VulkanResource* VBO = nullptr;
        glm::vec4 Color = glm::vec4(0.0f, 0.9f, 0.1f, 0.6f);
        std::vector<glm::vec3> positions;
        uint32_t vertexCount = 0;
        VkDeviceSize vboSize = 0;
    };

    void drawHelperMesh(VkCommandBuffer cmd, const helperMesh & mesh) const;
    void updateDragArc(const ArcballCameraController& arcball);
    void updateRimArc(const ArcballCameraController& arcball);
    void updateConstraintArcs(const ArcballCameraController& arcball);
    void updateResultArc(const ArcballCameraController& arcball);

    void fillConstraint(const ArcballCameraController& arcball, size_t idx);
    void fillCircle(const ArcballCameraController& arcball, std::vector<glm::vec3>& positions) const;
    void fillArc(const ArcballCameraController& arcball, std::vector<glm::vec3>& positions, glm::vec3 from, glm::vec3 to) const;
    void fillHalfArc(const ArcballCameraController& arcball, std::vector<glm::vec3>& positions, glm::vec3 axis) const;
    void updateMesh(helperMesh& mesh);
    glm::vec3 bisect(glm::vec3 a, glm::vec3 b) const;
    glm::vec4 axisIndexColor(size_t idx) const;

    void createPipelineLayout();
    void createShaders();
    void createPipelineCache();
    void createPipelines(VkRenderPass pass);

    const struct RendererContext_API* rendererApi;
    const struct ResourceContext_API* resourceApi;

    helperMesh drag;
    helperMesh rim;
    helperMesh result;
    std::array<helperMesh, 3> constraintMeshes;
    
    bool showDrag{ true };
    bool showRim{ true };
    bool overrideRim;
    bool showResult{ true };
    bool showConstraints{ true };
    bool usePointsPipeline;

    std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
    std::unique_ptr<vpr::ShaderModule> vert, frag;
    std::unique_ptr<vpr::PipelineCache> pipelineCache;
    VkPipeline linesPipeline;
    VkPipeline pointsPipeline;

};

#endif // !ARCBALL_HELPER_OBJECT_HPP

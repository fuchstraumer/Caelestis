#pragma once
#ifndef TERRAIN_PLUGIN_VIEW_FRUSTUM_HPP
#define TERRAIN_PLUGIN_VIEW_FRUSTUM_HPP
#include "glm/mat4x4.hpp"
#include <array>

struct AABB;

struct ViewFrustum {
    ViewFrustum();
    ViewFrustum(const float screen_depth, const glm::mat4& projection, const glm::mat4& view, const glm::mat4 world = glm::mat4(1.0f));
    
    void Construct(const float screen_depth, const glm::mat4& projection, const glm::mat4& view, const glm::mat4 world = glm::mat4(1.0f));
    bool ContainsPoint(const glm::vec3& p) const;
    bool ContainsSphere(const glm::vec3& p, const float& radius);
    bool ContainsAABB(const AABB& box);
    bool IntersectsAABB(const AABB& box);

private:
    std::array<glm::vec4, 6> planes;
    std::array<glm::vec4, 6> planeNormals;
    glm::mat4 view;
};

#endif //!TERRAIN_PLUGIN_VIEW_FRUSTUM_HPP

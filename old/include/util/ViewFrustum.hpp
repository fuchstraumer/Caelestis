#pragma once
#ifndef VPSK_VIEW_FRUSTUM_HPP
#define VPSK_VIEW_FRUSTUM_HPP
#include "AABB.hpp"
#include "UtilitySphere.hpp"
#include <array>
#include "glm/mat4x4.hpp"
#include "glm/fwd.hpp"
namespace vpsk {

    class ViewFrustum {
    public:

        ViewFrustum(const glm::mat4& view_projection_matrix);

        void BuildPlanes(const glm::mat4& view_projection_matrix);
        bool Intersects(const AABB& aabb) const noexcept;
        bool FastIntersection(const AABB& aabb) const noexcept;
        glm::vec3 GetCoordinate(const glm::vec3& d_pos) const noexcept;

        static UtilitySphere GetBoundingSphere(const glm::mat4& projection, const glm::mat4& vew);
    private:

        std::array<glm::vec4, 6> planes;
        glm::mat4 invViewProjection;
    };

}

#endif //!VPSK_VIEW_FRUSTUM_HPP
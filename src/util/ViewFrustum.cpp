#include "util/ViewFrustum.hpp"

namespace vpsk {

    static const std::array<glm::vec4, 8> helper_array {
        glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f),
        glm::vec4(-1.0f,-1.0f, 1.0f, 1.0f),
        glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),
        glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f),
        glm::vec4( 1.0f,-1.0f, 1.0f, 1.0f),
        glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f)
    };

    ViewFrustum::ViewFrustum(const glm::mat4& view_projection) {
        BuildPlanes(view_projection);
    }

    void ViewFrustum::BuildPlanes(const glm::mat4& vp) {

        invViewProjection = glm::inverse(vp);

        static const auto project = [](const glm::vec4& v)->glm::vec3 {
            return glm::vec3(v.x, v.y, v.z) / glm::vec3(v.w);
        };

        const glm::vec3 tln = project(invViewProjection * helper_array[0]);
        const glm::vec3 bln = project(invViewProjection * helper_array[2]);
        const glm::vec3 blf = project(invViewProjection * helper_array[3]);
        const glm::vec3 trn = project(invViewProjection * helper_array[4]);
        const glm::vec3 trf = project(invViewProjection * helper_array[5]);
        const glm::vec3 brn = project(invViewProjection * helper_array[6]);
        const glm::vec3 brf = project(invViewProjection * helper_array[7]);

        const glm::vec3 l = glm::normalize(glm::cross(blf - bln, tln - bln));
        const glm::vec3 r = glm::normalize(glm::cross(trf - trn, brn - trn));
        const glm::vec3 n = glm::normalize(glm::cross(bln - brn, trn - brn));
        const glm::vec3 f = glm::normalize(glm::cross(trf - brf, blf - brf));
        const glm::vec3 t = glm::normalize(glm::cross(tln - trn, trf - trn));
        const glm::vec3 b = glm::normalize(glm::cross(brf - brn, bln - brn));

        planes[0] = glm::vec4(l, -glm::dot(l, bln));
        planes[1] = glm::vec4(r, -glm::dot(r, trn));
        planes[2] = glm::vec4(n, -glm::dot(n, brn));
        planes[3] = glm::vec4(f, -glm::dot(f, brf));
        planes[4] = glm::vec4(t, -glm::dot(t, trn));
        planes[5] = glm::vec4(b, -glm::dot(b, brn));

    }

    bool ViewFrustum::Intersects(const AABB& aabb) const noexcept {
        for (const auto& p : planes) {
            bool intersects = false;
            for (size_t i = 0; i < 8; ++i) {
                const float dp = glm::dot(glm::vec4(aabb.GetCorner(i), 1.0f), p);
                if (dp >= 0.0f) {
                    intersects = true;
                    break;
                }
            }

            if (!intersects) {
                return false;
            }
        }
        return true;
    }

    bool ViewFrustum::FastIntersection(const AABB& aabb) const noexcept {
        const glm::vec4 center(aabb.Center(), 1.0f);
        const float radius = 0.5f * glm::distance(aabb.Min(), aabb.Max());
        for (const auto& p : planes) {
            if (glm::dot(p, center) < -radius) {
                return false;
            }
        }
        return true;
    }

    glm::vec3 ViewFrustum::GetCoordinate(const glm::vec3& d) const noexcept {
        glm::vec4 clip(2.0f * d.x - 1.0f, 2.0f * d.y - 1.0f, d.z, 1.0f);
        clip = invViewProjection * clip;
        return glm::vec3(clip.x, clip.y, clip.z) / clip.w;
    }

}
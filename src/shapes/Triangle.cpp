#include "shapes/Triangle.hpp"
#include "geometries/TriangleMesh.hpp"
#include "interaction/SurfaceInteraction.hpp"

namespace vpsk {

    constexpr float machine_epsilon = std::numeric_limits<float>::epsilon() * 0.5f;
    constexpr inline float gamma(const size_t& n) {
        return (static_cast<float>(n) * machine_epsilon) / (1.0f - static_cast<float>(n) * machine_epsilon);
    }

    glm::vec3 permute_vector(const glm::vec3& p, const size_t& i0, const size_t& i1, const size_t& i2) {
        return glm::vec3(p[i0], p[i1], p[i2]);
    }

    Triangle::Triangle(const TriangleMesh* _parent, const size_t& _i0, const size_t& _i1, const size_t& _i2) : 
        parent(_parent), i0(_i0), i1(_i1), i2(_i2) {}

    util::AABB Triangle::GetObjectAABB() const noexcept {
        const auto& v0 = parent->GetVertexPosition(i0);
        const auto& v1 = parent->GetVertexPosition(i1);
        const auto& v2 = parent->GetVertexPosition(i2);
        return util::AABB::Union(util::AABB(v0, v1), v2);
    }

    util::AABB Triangle::GetWorldAABB() const noexcept {
        const auto v0 = glm::vec3(glm::vec4(parent->GetVertexPosition(i0), 0.0f) * parent->GetModelMatrix());
        const auto v1 = glm::vec3(glm::vec4(parent->GetVertexPosition(i1), 0.0f) * parent->GetModelMatrix());
        const auto v2 = glm::vec3(glm::vec4(parent->GetVertexPosition(i2), 0.0f) * parent->GetModelMatrix());
        return util::AABB::Union(util::AABB(v0, v1), v2);
    }

    bool Triangle::Intersect(const Ray& ray, const float& t_max, float* t_hit, SurfaceInteraction* intersect) const {
        const auto v0 = glm::vec3(glm::vec4(parent->GetVertexPosition(i0), 0.0f) * parent->GetModelMatrix()) - ray.GetOrigin();
        const auto v1 = glm::vec3(glm::vec4(parent->GetVertexPosition(i1), 0.0f) * parent->GetModelMatrix()) - ray.GetOrigin();
        const auto v2 = glm::vec3(glm::vec4(parent->GetVertexPosition(i2), 0.0f) * parent->GetModelMatrix()) - ray.GetOrigin();

        const auto& k = ray.GetIdxVector();
        glm::vec3 pv0 = permute_vector(v0, k.x, k.y, k.z);
        glm::vec3 pv1 = permute_vector(v1, k.x, k.y, k.z);
        glm::vec3 pv2 = permute_vector(v2, k.x, k.y, k.z);

        const auto& s = ray.GetSVector();
        pv0.x += s.x * pv0.z;
        pv0.y += s.y * pv0.z;
        pv1.x += s.x * pv1.z;
        pv1.y += s.y * pv1.z;
        pv2.x += s.x * pv2.z;
        pv2.y += s.y * pv2.z;

        const glm::vec3 e = glm::vec3(
            pv1.x*pv2.y - pv1.y*pv2.x,
            pv2.x*pv0.y - pv2.y*pv0.x,
            pv0.x*pv1.y - pv0.y*pv1.x);
        
        if((e.x < 0.0f || e.y < 0.0f || e.z < 0.0f) && (e.x > 0.0f || e.y > 0.0f || e.z > 0.0f)) {
            return false;
        }

        const float det = e.x + e.y + e.z;
        if(det == 0.0f) {
            return false;
        }

        pv0.z *= s.z;
        pv1.z *= s.z;
        pv2.z *= s.z;

        const float t_scaled = e.x * pv0.z + e.y * pv1.z + e.z * pv2.z;
        if (det < 0.0f && (t_scaled >= 0.0f || t_scaled < t_max * det)) {
            return false;
        }
        else if (det > 0.0f && (t_scaled <= 0.0f || t_scaled > t_max * det)) {
            return false;
        }
        
        const float det_inv = 1.0f / det;
        const glm::vec3 b = e * det_inv;
        const float t = t_scaled * det_inv;

        const auto& uv0 = parent->GetVertexData(i0).uv;
        const auto& uv1 = parent->GetVertexData(i1).uv;
        const auto& uv2 = parent->GetVertexData(i2).uv;

        const glm::vec2 duv02 = uv0 - uv2;
        const glm::vec2 duv12 = uv1 - uv2;
        const glm::vec3 dp02 = v0 - v2;
        const glm::vec3 dp12 = v1 - v2;
        
        glm::vec3 dpdu, dpdv;
        const float determinant = duv02.x * duv12.y - duv02.y * duv12.x;
        if(std::abs(determinant) < 1.0e-7f) {
            const glm::vec3 cs = glm::normalize(glm::cross(v2 - v0, v1 - v0));
            if(std::abs(cs.x) > std::abs(cs.y)) {
                dpdu = glm::vec3(-cs.z, 0.0f, cs.x) / std::sqrt(cs.x*cs.x + cs.z*cs.z);
            }
            else {
                dpdu = glm::vec3(0.0f, cs.z, -cs.y) / std::sqrt(cs.y*cs.y + cs.z*cs.z);
            }
            dpdv = glm::cross(cs, dpdu);
        }
        else {
            const float invdet = 1.0f / determinant;
            dpdu = (duv12.y * dp02 - duv02.y * dp12) * invdet;
            dpdv = (-duv12.y * dp02 + duv02.x * dp12) * invdet;
        }

        const glm::vec3 p_error = glm::vec3(
            std::abs(b.x * v0.x) + std::abs(b.y * v1.x) + std::abs(b.z * v2.x) * gamma(7),
            std::abs(b.x * v0.y) + std::abs(b.y * v1.y) + std::abs(b.z * v2.y) * gamma(7),
            std::abs(b.x * v0.z) + std::abs(b.y * v1.z) + std::abs(b.z * v2.z) * gamma(7)
        );

        const glm::vec3 phit = b.x * v0 + b.y * v1 + b.z * v2;
        const glm::vec2 uvhit = b.x * uv0 + b.y * uv1 + b.z * uv2;
            
        *intersect = SurfaceInteraction(phit, glm::vec3(0.0f), p_error, -ray.GetDirection(), 
                                        0.0f, uvhit, dpdu, dpdv, glm::vec3(0.0f), glm::vec3(0.0f));

        intersect->Normal = intersect->shadingInfo.n = glm::normalize(glm::cross(dp02, dp12));
        // TODO: normal slope/delta calculation.
        return true;
    }

}
#pragma once
#ifndef VPSK_SURFACE_INTERACTION_HPP
#define VPSK_SURFACE_INTERACTION_HPP
#include "Interaction.hpp"
#include "glm/vec2.hpp"

namespace vpsk {

    class Primitive;

    class SurfaceInteraction : public Interaction {
    public:

        SurfaceInteraction(const glm::vec3& p, const glm::vec3& norm, const glm::vec3& p_error,
                           const glm::vec3& neg_ray_dir, const float& time, const glm::vec2& uv,
                           const glm::vec3& dpdu, const glm::vec3& dpdv, const glm::vec3& dndu, const glm::vec3& dndv);
        
        struct shading_info_t {
            glm::vec3 n;
            glm::vec3 dpdu, dpdv;
            glm::vec3 dndu, dndv;
        } shadingInfo;

        const Primitive* GetPrimitive() const noexcept;
        void SetPrimitive(const Primitive* _primitive) noexcept;

    protected:
        // Coordinates from surface parametrization.
        glm::vec2 uv;
        // Change in position and normal, relative to parameterization.
        glm::vec3 dpdu, dpdv;
        glm::vec3 dndu, dndv;
        const Primitive* primitive = nullptr;
    };

}

#endif //!VPSK_SURFACE_INTERACTION_HPP
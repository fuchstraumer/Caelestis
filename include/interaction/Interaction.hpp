#pragma once
#ifndef VPSK_INTERACTION_BASE_HPP
#define VPSK_INTERACTION_BASE_HPP
#include "glm/vec3.hpp"

namespace vpsk {

    // Back for this after we add scattering medium support.
    class MediumInterface;

    class Interaction {
    public:

        Interaction();
        Interaction(const glm::vec3& p, const glm::vec3& norm, const glm::vec3& p_error, 
                    const glm::vec3& neg_ray_dir, const float& time);

        bool IsSurfaceInteraction() const noexcept;

        float Time;
        glm::vec3 P, pError, NegRayDir;
        glm::vec3 Normal;
        
    protected:
        MediumInterface* medInterface = nullptr;
    };

}

#endif //!VPSK_INTERACTION_BASE_HPP
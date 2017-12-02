#include "interaction/Interaction.hpp"

namespace vpsk {

    Interaction::Interaction() : P(0.0f), NegRayDir(0.0f), Normal(0.0f), Time(0.0f), pError(0.0f) {}

    Interaction::Interaction(const glm::vec3& p, const glm::vec3& norm, const glm::vec3& p_error, 
                    const glm::vec3& w_0, const float& time) : P(p), Normal(norm), pError(p_error),
                    NegRayDir(w_0), Time(time) {}
    
    bool Interaction::IsSurfaceInteraction() const noexcept {
        return Normal != glm::vec3(0.0f,0.0f,0.0f);
    }

}
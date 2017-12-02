#include "interaction/SurfaceInteraction.hpp"

namespace vpsk {

    SurfaceInteraction::SurfaceInteraction(const glm::vec3& p, const glm::vec3& norm, const glm::vec3& p_error, const glm::vec3& neg_ray_dir, const float& time, 
        const glm::vec2& _uv, const glm::vec3& _dpdu, const glm::vec3& _dpdv, const glm::vec3& _dndu, const glm::vec3& _dndv) : Interaction(p, norm, p_error, neg_ray_dir, time),
        uv(_uv), dpdu(_dpdu), dpdv(_dpdv), dndu(_dndu), dndv(_dndv), shadingInfo({ norm, _dpdu, _dpdv, _dndu, _dndv }) {}

}
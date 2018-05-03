#pragma once
#ifndef VPSK_SPOT_LIGHT_HPP
#define VPSK_SPOT_LIGHT_HPP
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
namespace vpsk {

    struct alignas(16) SpotLight {
        glm::vec4 Position{ glm::vec3(0.0f), 1.0f };
        glm::vec4 ViewSpacePosition{ glm::vec3(0.0f), 1.0f };
        glm::vec4 Direction{ 0.0f, 0.0f, 1.0f, 0.0f };
        glm::vec4 ViewSpaceDirection{ 0.0f, 0.0f,-1.0f, 0.0f };
        glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
        float SpotlightAngle{ 45.0f };
        float Range{ 100.0f };
        float Intensity{ 1.0f };
        uint32_t Enabled{ 1 };
        uint32_t Selected{ 0 };
    };
}

#endif //!VPSK_SPOT_LIGHT_HPP
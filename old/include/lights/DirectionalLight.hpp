#pragma once
#ifndef VPSK_DIRECTIONAL_LIGHT_HPP
#define VPSK_DIRECTIONAL_LIGHT_HPP
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace vpsk {

    struct alignas(16) DirectionalLight {
        glm::vec4 Direction{ 0.0f, 0.0f, 1.0f, 0.0f };
        glm::vec4 DirectionViewSpace{ 0.0f, 0.0f,-1.0f, 0.0f};
        glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
        float Intensity{ 1.0f };
        uint32_t Enabled{ 1 };
        uint32_t Selected{ 0 };
        glm::vec2 Padding{ 0.0f, 0.0f };
    };

}

#endif //!VPSK_DIRECTIONAL_LIGHT_HPP
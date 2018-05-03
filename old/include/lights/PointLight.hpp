#pragma once
#ifndef VPSK_POINT_LIGHT_HPP
#define VPSK_POINT_LIGHT_HPP
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
namespace vpsk {

    struct alignas(16) PointLight {
        glm::vec4 Position{ 0.0f, 0.0f, 0.0f, 1.0f };
        glm::vec4 PositionViewSpace{ 0.0f, 0.0f, 0.0f, 1.0f };
        glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
        float Range{ 100.0f };
        float Intensity{ 1.0f };
        uint32_t Enabled{ 1 };
        uint32_t Selected{ 0 };
        float Padding{ 0.0f };
    };

}

#endif //!VPSK_POINT_LIGHT_HPP
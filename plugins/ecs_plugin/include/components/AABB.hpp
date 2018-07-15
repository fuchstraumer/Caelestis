#pragma once
#ifndef VPSK_AABB_COMPONENT_HPP
#define VPSK_AABB_COMPONENT_HPP
#include "glm/vec4.hpp"

namespace vpsk {

    struct AABB {
        glm::vec4 Max = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        glm::vec4 Min = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    };

}

#endif //!VPSK_AABB_COMPONENT_HPP
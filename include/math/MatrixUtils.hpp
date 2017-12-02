#pragma once
#ifndef VULPES_VK_MATRIX_UTILS_HPP
#define VULPES_VK_MATRIX_UTILS_HPP
#include "glm/vec3.hpp"
#include "glm/fwd.hpp"

namespace vpsk {

    glm::mat4 AlignZAxisWithTarget(glm::vec3& target_direction, glm::vec3& up_direction);

}
#endif // !VULPES_VK_MATRIX_UTILS_HPP

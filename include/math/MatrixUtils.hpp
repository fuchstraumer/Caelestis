#pragma once
#ifndef VULPES_VK_MATRIX_UTILS_HPP
#define VULPES_VK_MATRIX_UTILS_HPP
#include "math/matrix_float4x4.hpp"

namespace vpsk {

    mango::float4x4 AlignZAxisWithTarget(mango::float32x3& target_direction, mango::float32x3& up_direction);

}
#endif // !VULPES_VK_MATRIX_UTILS_HPP

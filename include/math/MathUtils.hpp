#pragma once
#ifndef VULPES_MATH_UTILITIES_HPP
#define VULPES_MATH_UTILITIES_HPP
#include "math/vector_float32x2.hpp"

namespace vpsk {
    
    float PointOnEllipseBisector(const size_t& num_components, const mango::float32x2& extents, const mango::float32x2& y, 
        mango::float32x2& x);
    float PointOnEllipseSqrDistanceImpl(const mango::float32x2& extents, const mango::float32x2& y, mango::float32x2& x);
    float PointOnEllipseSqrDistance(const mango::float32x2& extents, const mango::float32x2& y, mango::float32x2& x);
    mango::float32x2 GetClosestPointOnEllipse(const mango::float32x2& center, const mango::float32x2& axis_a, 
        const mango::float32x2& axis_b, const mango::float32x2& test_point);

}

#endif // !VULPES_MATH_UTILITIES_HPP

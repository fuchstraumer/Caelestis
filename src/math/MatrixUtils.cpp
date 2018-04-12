#include "math/MatrixUtils.hpp"
#include <array>

namespace vpsk {

    mango::float4x4 AlignZAxisWithTarget(mango::float32x3 & target_direction, mango::float32x3& up_direction) {

        if (mango::length(target_direction) == 0.0f) {
            target_direction = mango::float32x3{ 0.0f, 0.0f, 1.0f };
        }

        if (mango::length(up_direction) == 0.0f) {
            up_direction = mango::float32x3{ 0.0f, 1.0f, 0.0f };
        }

        if (mango::length(mango::cross(up_direction, target_direction)) == 0.0f) {
            up_direction = mango::cross(target_direction, mango::float32x3{ 1.0f, 0.0f, 0.0f });
        }
        if (mango::length(up_direction) == 0.0f) {
            up_direction = mango::cross(target_direction, mango::float32x3{ 0.0f, 0.0f, 1.0f });
        }

        mango::float32x3 target_perspective_dir = mango::cross(up_direction, target_direction);
        mango::float32x3 target_up_dir = mango::cross(target_direction, target_perspective_dir);

        std::array<mango::float32x3, 3> matrix_rows{
            mango::normalize(target_perspective_dir),
            mango::normalize(target_up_dir),
            mango::normalize(target_direction)
        };

        const std::array<float, 16> result{
            matrix_rows[0].x, matrix_rows[0].y, matrix_rows[0].z, 0.0f,
            matrix_rows[1].x, matrix_rows[1].y, matrix_rows[1].z, 0.0f,
            matrix_rows[2].x, matrix_rows[2].y, matrix_rows[2].z, 0.0f,
            0.0f,             0.0f,             0.0f, 1.0f,
        };

        return mango::float4x4(result.data());
    }


}
#include "math/MathUtils.hpp"
#include <algorithm>

namespace vpsk {

    float PointOnEllipseBisector(const size_t & num_components, const mango::float32x2 & extents, const mango::float32x2 & y,
         mango::float32x2 & x) {
        
        mango::float32x2 z;
        float sum_z_sq = 0.0f;
        size_t i;
        for (i = 0; i < num_components; ++i) {
            z[i] = y[i] / extents[i];
            sum_z_sq += z[i] * z[i];
        }

        if (sum_z_sq == 1.0f) {
            for (i = 0; i < num_components; ++i) {
                x[i] = y[i];
            }
            return 0.0f;
        }

        float e_min = extents[static_cast<size_t>(num_components - 1)];
        mango::float32x2 p_sqr, numerator;
        for (i = 0; i < num_components; ++i) {
            float p = extents[i] / e_min;
            p_sqr[i] = p * p;
            numerator[i] = p_sqr[i] * z[i];
        }

        const size_t j_max = static_cast<size_t>(std::numeric_limits<float>::digits - std::numeric_limits<float>::min_exponent);
        float s_min = z[static_cast<size_t>(num_components - 1)];
        float s_max;

        if (sum_z_sq < 1.0f) {
            s_max = 0.0f;
        }
        else {
            s_max = mango::length(numerator) - 1.0f;
        }

        float s = 0.0f;
        for (size_t j = 0; j < j_max; ++j) {
            s = (s_min + s_max) * 0.50f;
            if (s == s_min || s == s_max) {
                break;
            }

            float g = -1.0f;
            for (i = 0; i < num_components; ++i) {
                float ratio = numerator[i] / (s + p_sqr[i]);
                g += ratio * ratio;
            }

            if (g > 0.0f) {
                s_min = s;
            }
            else if (g < 0.0f) {
                s_max = s;
            }
            else {
                break;
            }
        }

        float sqr_distance = 0.0f;
        for (i = 0; i < num_components; ++i) {
            x[i] = p_sqr[i] * y[i] / (s + p_sqr[i]);
            float diff = x[i] - y[i];
            sqr_distance += diff * diff;
        }

        return sqr_distance;
    }

    float PointOnEllipseSqrDistanceImpl(const mango::float32x2 & extents, const mango::float32x2 & y, mango::float32x2 & x) {
        float sqr_distance = 0.0f;

        mango::float32x2 e_pos, y_pos, x_pos;
        size_t num_pos = 0;
        for (size_t i = 0; i < 2; ++i) {
            if (y[i] > 0.0f) {
                e_pos[num_pos] = extents[i];
                y_pos[num_pos] = y[i];
                ++num_pos;
            }
            else {
                x[i] = 0.0f;
            }
        }

        if (y[1] > 0.0f) {
            sqr_distance = PointOnEllipseBisector(num_pos, e_pos, y_pos, x_pos);
        }
        else {
            float numerator[1], denominator[1];
            float e_nm_sqr = extents[1] * extents[1];
            for (size_t i = 0; i < num_pos; ++i) {
                numerator[i] = e_pos[i] * y_pos[i];
                denominator[i] = e_pos[i] * e_pos[i] - e_nm_sqr;
            }

            bool in_sub_hyperbox = true;
            for (size_t i = 0; i < num_pos; ++i) {
                if (numerator[i] >= denominator[i]) {
                    in_sub_hyperbox = false;
                    break;
                }
            }

            bool in_sub_hyperellipsoid = false;
            if (in_sub_hyperbox) {
                float xde[1];
                float discr = 1.0f;
                for (size_t i = 0; i < num_pos; ++i) {
                    xde[i] = numerator[i] / denominator[i];
                    discr -= xde[i] * xde[i];
                }
                if (discr > 0.0f) {
                    sqr_distance = 0.0f;
                    for (size_t i = 0; i < num_pos; ++i) {
                        x_pos[i] = e_pos[i] * xde[i];
                        float diff = x_pos[i] - y_pos[i];
                        sqr_distance += diff * diff;
                    }
                    x[1] = extents[1] * sqrtf(discr);
                    sqr_distance += x[1] * x[1];
                    in_sub_hyperellipsoid = true;
                }
            }

            if (!in_sub_hyperellipsoid) {
                x[1] = 0.0f;
                sqr_distance = PointOnEllipseBisector(num_pos, e_pos, y_pos, x_pos);
            }
        }

        for (size_t i = 0, num_pos = 0; i < 2; ++i) {
            if (y[i] > 0.0f) {
                x[i] = x_pos[num_pos];
                ++num_pos;
            }
        }

        return sqr_distance;
    }

    float PointOnEllipseSqrDistance(const mango::float32x2 & extents, const mango::float32x2 & y, mango::float32x2 & x) {
        bool negation[2];
        negation[0] = y.x < 0.0f;
        negation[1] = y.y < 0.0f;

        std::pair<float, int> permutations[2];
        permutations[0] = std::make_pair(-extents.x, 0);
        permutations[1] = std::make_pair(-extents.y, 1);
        std::sort(&permutations[0], &permutations[1]);

        int inverse_permutations[2];
        for (size_t i = 0; i < 2; ++i) {
            inverse_permutations[permutations[i].second] = i;
        }

        mango::float32x2 location_e, location_y;
        int j;

        for (size_t i = 0; i < 2; ++i) {
            j = permutations[i].second;
            location_e[i] = extents[j];
            location_y[i] = std::abs(y[j]);
        }

        mango::float32x2 location_x;
        float sqr_distance = PointOnEllipseSqrDistanceImpl(location_e, location_y, location_x);

        for (size_t i = 0; i < 2; ++i) {
            j = inverse_permutations[i];
            if (negation[i]) {
                location_x[j] = -location_x[j];
            }
            x[i] = location_x[j];
        }

        return sqr_distance;
    }

    mango::float32x2 GetClosestPointOnEllipse(const mango::float32x2 & center, const mango::float32x2 & axis_a, 
        const mango::float32x2 & axis_b, const mango::float32x2 & test_point) {
        const float length_a = mango::length(axis_a);
        const float length_b = mango::length(axis_b);

        const mango::float32x2 unit_a = axis_a / length_a;
        const mango::float32x2 unit_b = axis_b / length_b;
        const mango::float32x2 diff = test_point - center;
        const mango::float32x2 y = mango::float32x2(mango::dot(diff, unit_a), mango::dot(diff, unit_b));

        mango::float32x2 x;
        const mango::float32x2 extents(length_a, length_b);
        PointOnEllipseSqrDistance(extents, y, x);

        mango::float32x2 result = center;
        result += x[0] * unit_a;
        result += x[1] * unit_b;
        return result;
    }

}
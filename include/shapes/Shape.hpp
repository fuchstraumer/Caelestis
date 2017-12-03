#pragma once
#ifndef VPSK_SHAPE_HPP
#define VPSK_SHAPE_HPP
#include "util/AABB.hpp"
#include "math/Ray.hpp"
#include <memory>

namespace vpsk {

    class SurfaceInteraction;

    class Shape {
    public:

        Shape() = default;
        ~Shape(){};
        virtual bool Intersect(const Ray& ray, const float& t_max, float* t_hit, SurfaceInteraction* intersect) const noexcept = 0;
        virtual bool Intersect(const Ray& ray, const float& t_max) const noexcept { return Intersect(ray, t_max, nullptr, nullptr); }
        virtual float Area() const noexcept = 0;
        virtual util::AABB GetObjectAABB() const noexcept = 0;
        virtual util::AABB GetWorldAABB() const noexcept = 0;
    protected:

    };

}

#endif //!VPSK_SHAPE_HPP
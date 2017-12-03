#pragma once
#ifndef VPSK_TRIANGLE_SHAPE_HPP
#define VPSK_TRIANGLE_SHAPE_HPP
#include "Shape.hpp"

namespace vpsk {

    class TriangleMesh;

    class Triangle : public Shape {
    public:

        Triangle(const TriangleMesh* _parent, const size_t& i0,
            const size_t& i1, const size_t& i2);
        
        float Area() const noexcept final;
        bool Intersect(const Ray& ray, const float& t_max, float* t_hit, SurfaceInteraction* surf) const noexcept final;
        util::AABB GetObjectAABB() const noexcept final;
        util::AABB GetWorldAABB() const noexcept final;

    protected:
        const TriangleMesh* parent;
        size_t i0, i1, i2;
    };

}

#endif //!VPSK_TRIANGLE_SHAPE_HPP
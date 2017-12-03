#pragma once
#ifndef VPSK_PRIMITIVE_BASE_HPP
#define VPSK_PRIMITIVE_BASE_HPP
#include "util/AABB.hpp"
#include "math/Ray.hpp"

namespace vpsk {

    /**
     * The primitive classes represent the interface between geometry and shading: the base class defines a base interface 
     * required for rendering, such as tests for ray intersections and querying of material and lighting properties. Importantly,
     * this simple/bare interface makes it easier to in turn use and interface with acceleration structures like a BVH or a KD tree
     * for improving the performance of raycasting algorithms. 
     * \defgroup Primitives
    */ 

    /**
     * The primitive class itself is a pure-virtual class that merely defines the interface required for generating the data we need
     * for any given renderable object (usually, something from the geometries group.)
     * \ingroup Primitives
    */ 

    class AreaLight;
    class Material;
    class SurfaceInteraction;

    class Primitive {
        Primitive(const Primitive&) = delete;
        Primitive& operator=(const Primitive&) = delete;
    public:
        virtual ~Primitive();
        virtual util::AABB WorldBounds() const noexcept = 0;
        virtual util::AABB ObjBounds() const noexcept = 0;
        virtual bool Intersect(const Ray& _r, SurfaceInteraction* interaction) const noexcept = 0;
        virtual bool Intersect(const Ray& _r) const noexcept = 0;
    };
}

#endif //!VPSK_PRIMITIVE_BASE_HPP
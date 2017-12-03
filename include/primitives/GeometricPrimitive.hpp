#pragma once
#ifndef VPSK_GEOMETRIC_PRIMITIVE_HPP
#define VPSK_GEOMETRIC_PRIMITIVE_HPP
#include "Primitive.hpp"
#include <memory>
namespace vpsk {

    /**
     * The GeometricPrimitive class is a specialization of the Primitive class intended specifically for use
     * with one of the geometry classes, generally for rendering objects using conventional draw commands
     * or objects that are not frequently rendered.
     * \ingroup Primitives
    */

    class Material;
    class TriangleMesh;
    class AreaLight;

    class GeometricPrimitive : public Primitive {
    public:

        util::AABB WorldBounds() const noexcept final;
        util::AABB ObjBounds() const noexcept final;
        bool Intersect(const Ray& _r, SurfaceInteraction* interaction) const noexcept final;
        bool Intersect(const Ray& _r) const noexcept final;
        
        const TriangleMesh* GetMesh() const noexcept;
        const AreaLight* GetAreaLight() const noexcept;
        const Material* GetMaterial() const noexcept;
    private:
        const TriangleMesh* mesh;
        const AreaLight* light = nullptr;
        const Material* material = nullptr;
    };
}

#endif //!VPSK_GEOMETRIC_PRIMITIVE_HPP

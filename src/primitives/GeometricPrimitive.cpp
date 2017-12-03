#include "primitives/GeometricPrimitive.hpp"
#include "lights/Area.hpp"
#include "geometries/TriangleMesh.hpp"

namespace vpsk {
    
    util::AABB GeometricPrimitive::WorldBounds() const noexcept {
        util::AABB result;
        result.Transform(mesh->GetModelMatrix());
        return result;
    }

    util::AABB GeometricPrimitive::ObjBounds() const noexcept {
        return util::AABB();
    }

    const TriangleMesh* GeometricPrimitive::GetMesh() const noexcept {
        return mesh;
    }

    const AreaLight* GeometricPrimitive::GetAreaLight() const noexcept {
        return light;
    }

    const Material* GeometricPrimitive::GetMaterial() const noexcept {
        return material;
    }

}
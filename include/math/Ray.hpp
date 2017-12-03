#pragma once
#ifndef VULPESRENDER_RAY_HPP
#define VULPESRENDER_RAY_HPP
#include "glm/vec3.hpp"
#include "glm/fwd.hpp"

namespace vpsk {

    class Ray {
    public:

        Ray() = default;
        Ray(const glm::vec3& _origin, const glm::vec3& _dir);

        const glm::vec3& GetOrigin() const noexcept;
        const glm::vec3& GetDirection() const noexcept;
        const glm::vec3& GetInvDirection() const noexcept;

        void SetDirection(const glm::vec3& new_direction) noexcept;
        void SetOrigin(const glm::vec3& new_origin) noexcept;

        void TransformRay(const glm::mat4& transform_matrix) noexcept;
        Ray GetTransformedRay(const glm::mat4& transform_matrix) const noexcept;

        glm::vec3 CalculatePosition(const float& t) const noexcept;

        bool CheckTriangleIntersection(const glm::vec3& vert_pos0, const glm::vec3& vert_pos1, const glm::vec3& vert_pos2, float* distance_to_intersection = nullptr) const noexcept;
        bool CheckPlaneIntersection(const glm::vec3& plane_origin, const glm::vec3& plane_normal, float* distance_to_intersection = nullptr) const noexcept;

        const glm::ivec3& GetIdxVector() const noexcept;
        const glm::vec3& GetSVector() const noexcept;
        
    protected:

        void setIdxVector() const;
        void setSVector() const;

        mutable glm::ivec3 idxVector;
        mutable bool idxVectorSet = false;
        mutable glm::vec3 sVector;
        mutable bool sVectorSet = false;
        glm::vec3 origin, direction, invDirection;
        char signX, signY, signZ;
    };
    
}
#endif // !VULPESRENDER_RAY_HPP

#pragma once
#ifndef SHARED_PRIMITIVES_AABB_HPP
#define SHARED_PRIMITIVES_AABB_HPP
#include "glm/vec3.hpp"
#include <cstdint>
#include <cstddef>
#include <array>

struct Ray;
struct Sphere;

struct AABB {

    AABB();
    AABB(glm::vec3 min, glm::vec3 max);
    ~AABB();
    AABB(const AABB& other) noexcept;
    AABB(AABB&& other) noexcept;
    AABB& operator=(const AABB& other) noexcept;
    AABB& operator=(AABB&& other) noexcept;

    static AABB MergeAABBs(const AABB& a, const AABB& b);

    float Area() const noexcept;
    float Radius() const noexcept;
    glm::vec3 Center() const noexcept;

    enum class IntersectionType : uint8_t {
        Outside,
        Intersects,
        Inside
    };

    IntersectionType Intersects(const AABB& other) const noexcept;
    IntersectionType Intersects(const Ray& r) const noexcept;

    std::array<glm::vec3, 8> Corners;
    glm::vec3 Min;
    glm::vec3 Max;

};

#endif //!SHARED_PRIMITIVES_AABB_HPP

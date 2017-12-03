#pragma once
#ifndef VULPES_UTIL_AABB_H
#define VULPES_UTIL_AABB_H
#include "glm/vec3.hpp"
#include "math/Ray.hpp"
namespace vpsk {

    namespace util {

        class AABB {
        public:

            AABB() = default;
            AABB(const glm::vec3& p);
            AABB(const glm::vec3& p0, const glm::vec3& p1);
            AABB(const glm::vec3& center, const float& radius);
            AABB(const AABB& other) noexcept;
            AABB(AABB&& other) noexcept;
            AABB& operator=(const AABB& other) noexcept;
            AABB& operator=(AABB&& other) noexcept;

            static AABB Union(const AABB& initial_bounds, const glm::vec3& point_to_include);
            static AABB Intersection(const AABB& b0, const AABB& b1);

            glm::vec3 Extents() const noexcept;
            glm::vec3 Center() const noexcept;
            bool Overlaps(const AABB& other) const noexcept;
            bool Contains(const glm::vec3& pt) const noexcept;
            const glm::vec3& Min() const noexcept;
            const glm::vec3& Max() const noexcept;

            void Include(const glm::vec3& pt);
            void Expand(const float& dist);
            bool Intersection(const glm::vec3& origin, const glm::vec3& ray) const;
            bool Intersection(const Ray& ray, float& t0, float& t1) const;
            glm::vec3 Diagonal() const noexcept;
            float SurfaceArea() const noexcept;
            float Volume() const noexcept;

            size_t IdxToLargestAxis() const noexcept;
            glm::vec3 Lerp(const glm::vec3& amt) const noexcept;

            void Transform(const glm::mat4& transformation);
            
        private:

            glm::vec3 min, max;
        };

        

    }

}

#endif // !VULPES_UTIL_AABB_H

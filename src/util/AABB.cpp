#include "util/AABB.hpp"
#include <algorithm>
#include <cmath>

namespace vpsk {


        const float lerp(const float& s, const float& v0, const float& v1) {
            return std::fma(s, v1, std::fma(-s, v0, v0));
        }

        AABB::AABB() : min(0.0f,0.0f,0.0f), max(0.0f,0.0f,0.0f) {}

        AABB::AABB(const glm::vec3& p) : min(p), max(p) {}

        AABB::AABB(const glm::vec3& p0, const glm::vec3& p1) :
            min(std::min(p0.x, p1.x), std::min(p0.y, p1.y), std::min(p0.z, p1.z)),
            max(std::max(p0.x, p1.x), std::max(p0.y, p1.y), std::max(p0.z, p1.z)) {}

        AABB::AABB(const glm::vec3& center, const float& radius) : AABB(center - radius, center + radius) {}

        AABB::AABB(const AABB& other) noexcept : min(other.Min()), max(other.Max()) {}

        AABB::AABB(AABB&& other) noexcept : min(std::move(other.min)), max(std::move(other.max)) {}

        AABB& AABB::operator=(const AABB& other) noexcept {
            min = other.Min();
            max = other.Max();
            return *this;
        }

        AABB& AABB::operator=(AABB&& other) noexcept {
            min = std::move(other.min);
            max = std::move(other.max);
            return *this;
        }

        AABB AABB::Union(const AABB& b, const glm::vec3& p) {
            return AABB(glm::vec3(std::min(b.Min().x, p.x), std::min(b.Min().y, p.y), std::min(b.Min().z, p.z)),
                        glm::vec3(std::max(b.Max().x, p.x), std::max(b.Max().y, p.y), std::max(b.Max().z, p.z)));
        }

        glm::vec3 AABB::Extents() const noexcept
        {
            return glm::vec3(max - min);
        }

        glm::vec3 AABB::Center() const noexcept {
            return (min + max) / 2.0f;
        }

        bool AABB::Overlaps(const AABB& b) const noexcept {
            bool x = (max.x >= b.Min().x) && (min.x <= b.Max().x);
            bool y = (max.y >= b.Min().y) && (min.y <= b.Max().y);
            bool z = (max.z >= b.Min().z) && (min.z <= b.Max().z);
            return (x && y && z);
        }

        bool AABB::Contains(const glm::vec3& pt) const noexcept {
            bool x = (pt.x >= min.x) && (pt.x <= max.x);
            bool y = (pt.y >= min.y) && (pt.y <= max.y);
            bool z = (pt.z >= min.z) && (pt.z <= max.z);
            return (x && y && z);
        }

        const glm::vec3& AABB::Min() const noexcept {
            return min;
        }

        const glm::vec3& AABB::Max() const noexcept  {
            return max;
        }

        void AABB::Include(const glm::vec3& pt) {
            *this = std::move(Union(*this, pt));
        }

        void AABB::Expand(const float& dist) {
            min -= dist;
            max += dist;
        }

        bool AABB::Intersection(const glm::vec3& origin, const glm::vec3& ray) const {
            
            glm::vec3 inv_ray = glm::vec3(1.0f) / ray;
            
            float txmin, txmax, tymin, tymax, tzmin, tzmax;
            txmin = (min.x - origin.x) * inv_ray.x;
            txmax = (max.x - origin.x) * inv_ray.x;
            tymin = (min.y - origin.y) * inv_ray.y;
            tymax = (max.y - origin.y) * inv_ray.y;
            tzmin = (min.z - origin.z) * inv_ray.z;
            tzmax = (max.z - origin.z) * inv_ray.z;

            float tmin = std::max(std::max(std::min(txmin, txmax), std::min(tymin, tymax)), std::min(tzmin, tzmax));
            float tmax = std::min(std::min(std::max(txmin, txmax), std::max(tymin, tymax)), std::max(tzmin, tzmax));
            
            if (tmax < 0.0f) {
                return false;
            }

            if (tmin > tmax) {
                return false;
            }

            return true;

        }

        glm::vec3 AABB::Diagonal() const noexcept {
            return max - min;
        }

        float AABB::SurfaceArea() const noexcept {
            const glm::vec3 diag = Diagonal();
            return 2.0f * (diag.x * diag.y + diag.x * diag.z + diag.y * diag.z);
        }

        float AABB::Volume() const noexcept {
            const glm::vec3 diag = Diagonal();
            return diag.x * diag.y * diag.z;
        }

        size_t AABB::IdxToLargestAxis() const noexcept {
            const glm::vec3 d = Diagonal();
            if (d.x > d.y && d.x > d.z) {
                return 0;
            }
            else if (d.y > d.z) {
                return 1;
            }
            else {
                return 2;
            }
        }

        glm::vec3 AABB::Lerp(const glm::vec3& p) const noexcept {
            return glm::vec3(lerp(p.x, min.x, max.x),
                             lerp(p.y, min.y, max.y),
                             lerp(p.z, min.z, max.z));
        }

        glm::vec3 AABB::GetCorner(const size_t i) const noexcept {
            float x = i & 1 ? max.x : min.x;
            float y = i & 2 ? max.y : min.y;
            float z = i & 4 ? max.z : min.z;
            return glm::vec3(x,y,z);
        }

}


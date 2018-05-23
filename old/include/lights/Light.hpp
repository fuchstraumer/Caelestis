#pragma once
#ifndef VPSK_LIGHT_HPP
#define VPSK_LIGHT_HPP
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/type_precision.hpp"
namespace vpsk {

    // Helps simulate light diffusion and propagation in spherical areas
    struct AreaSphere {
        AreaSphere() = default;
        AreaSphere(const glm::vec3 rr) : radii(std::move(rr)) {}
        AreaSphere(const float r, const float phi, const float theta) :
            radii{ std::move(r), std::move(phi), std::move(theta) } {}
        void Set(const glm::vec3& v) {
            radii.x = glm::length(v);
            if (v.x == 0.0f) {
                radii.y = 0.0f;
                radii.z = 0.0f;
                return;
            }
            else {
                radii.y = acosf(glm::clamp(-v.y / radii.x, -1.0f, 1.0f));
                radii.z = atan2f(-v.z, v.x);
            }
        }

        void RestrictPhi() {
            constexpr static float local_eps = 0.0001f;
            constexpr static float local_pi = 3.1415926f - local_eps;
            radii.y = std::fmaxf(local_eps, std::fmin(local_pi, radii.y));
        }

        glm::vec3 Get() const noexcept {
            return glm::vec3{ 
                radii.x * sinf(radii.y) * cosf(radii.z),
                radii.x * cosf(radii.y),
                radii.x * sinf(radii.y) * sinf(radii.z)
            };
        }

        glm::vec3 radii{ 0.0f, 0.0f, 0.0f };
    };

    struct Light {
        Light(const glm::vec3 pos, const glm::u8vec4 color, const float range) :
            Position(std::move(pos)), Color(std::move(color)), Range(std::move(range)) {}
        Light(const glm::vec3 pos, const Light& other) : Position(std::move(pos)),
            Color(other.Color), Range(other.Range) {}

        void Update(const float elapsed_time) {
            Sphere.Set(Position - Center);
            Sphere.radii.z += 0.001f;
            const glm::vec3 v = Sphere.Get();
            Position = Center + v;
        } 

        glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
        float Range;
        glm::u8vec4 Color{ 255, 255, 255, 255 };
        glm::vec3 Center { 0.0f, 0.0f, 0.0f }; 
        AreaSphere Sphere;
    };

}

#endif //!VPSK_LIGHT_HPP
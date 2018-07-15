#pragma once
#ifndef CLUSTERED_FORWARD_HPP
#define CLUSTERED_FORWARD_HPP
#include "RendererCore.hpp"
#include "core/Instance.hpp"
#include "core/LogicalDevice.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/Buffer.hpp"
#include "render/Swapchain.hpp"
#include "command/CommandPool.hpp"
#include "command/TransferPool.hpp"
#include "render/GraphicsPipeline.hpp"
#include "render/Renderpass.hpp"
#include "render/Framebuffer.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/PipelineCache.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/DescriptorSet.hpp"
#include "render/DepthStencil.hpp"
#include "core/ShaderPack.hpp"
#include <memory>
#include <map>
#include <random>
#include <deque>
#include <experimental/filesystem>
#include "ext/include/glm/glm.hpp"

struct AABB {
    glm::vec3 Min;
    glm::vec3 Max;
};

struct lights_soa_t {
    void update(const AABB& bounds) {

        const auto set_from_vec = [](const glm::vec4& v) {
            glm::vec3 el(0.0f);
            el.x = glm::length(v);
            if (v.x != 0.0f) {
                el.y = acosf(glm::clamp(-v.y / el.x, -1.0f, 1.0f));
                el.z = atan2f(-v.z, v.x);
            }
            return el;
        };

        const auto get_vec = [](const glm::vec3& v) {
            return glm::vec3{
                v.x * sinf(v.y) * cos(v.z),
                -v.x * cosf(v.y),
                -v.x * sinf(v.y) * sinf(v.z)
            };
        };

        const auto restrict_phi = [](const float& y) {
            return std::fmaxf(0.0001f, std::fminf(3.14159f - 0.0001f, y));
        };

        for (size_t i = 0; i < Positions.size(); ++i) {
            auto el = set_from_vec(Positions[i]);
            el.z += 0.001f;
            el.y = restrict_phi(el.y);
            auto v = get_vec(el);
            
            Positions[i].x = v.x;
            Positions[i].y = v.y;
            Positions[i].z = v.z;
        }

    }
    std::vector<glm::vec4> Positions;
    std::vector<glm::u8vec4> Colors;
} Lights;

std::uniform_real_distribution<float> rand_distr(0.0f, 1.0f);
std::random_device rd;
std::mt19937 rng(rd());

float random_unit_float() {
    return rand_distr(rng);
}

float random_range(const float l, const float h) {
    return l + (h - l) * random_unit_float();
}

glm::vec3 hue_to_rgb(const float hue) {
    const float s = hue * 6.0f;
    const float r0 = glm::clamp(s - 4.0f, 0.0f, 1.0f);
    const float g0 = glm::clamp(s - 0.0f, 0.0f, 1.0f);
    const float b0 = glm::clamp(s - 2.0f, 0.0f, 1.0f);
    const float r1 = glm::clamp(2.0f - s, 0.0f, 1.0f);
    const float g1 = glm::clamp(4.0f - s, 0.0f, 1.0f);
    const float b1 = glm::clamp(6.0f - s, 0.0f, 1.0f);

    return glm::vec3{ r0 + r1, g0 * g1, b0 * b1 };
}

glm::u8vec4 float_to_uchar(const glm::vec3& color) {
    return glm::u8vec4{ 
        static_cast<uint8_t>(std::round(color.x * 255.0f)),
        static_cast<uint8_t>(std::round(color.y * 255.0f)),
        static_cast<uint8_t>(std::round(color.z * 255.0f)),
        0
    };
}

void GenerateLights(const AABB& model_bounds) {
    const glm::vec3 extents = model_bounds.Extents();
    const float volume = extents.x * extents.y * extents.z;
    const float light_vol = volume / static_cast<float>(1024);
    const float base_range = powf(light_vol, 1.0f / 3.0f);
    const float max_range = base_range * 3.0f;
    const float min_range = base_range / 1.5f;
    const glm::vec3 half_size = (model_bounds.Max - model_bounds.Min) * 0.50f;
    const float pos_radius = std::max(half_size.x, std::max(half_size.y, half_size.z));
    for (uint32_t i = 0; i < 1024; ++i) {
        glm::vec3 fcol = hue_to_rgb(random_range(0.0f,1.0f));
        fcol *= 1.30f;
        fcol -= 0.15f;
        const glm::u8vec4 color = float_to_uchar(fcol);
        const glm::vec4 position{ 
            random_range(-pos_radius, pos_radius), random_range(-pos_radius, pos_radius), random_range(-pos_radius, pos_radius), random_range(min_range, max_range)
        };
        Lights.Positions.push_back(position);
        Lights.Colors.push_back(color);
    }
}

#endif //!CLUSTERED_FORWARD_HPP

#pragma once
#ifndef VPSK_TRANSFORM_COMPONENT_HPP
#define VPSK_TRANSFORM_COMPONENT_HPP
#define GLM_SWIZZLE
#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"

namespace vpsk {

    struct WorldTransformComponent {

        WorldTransformComponent(glm::vec3 p, glm::vec3 s = glm::vec3(1.0f), glm::vec3 rot = glm::vec3(0.0f));

        const glm::mat4& operator()() const;
        const glm::mat4& TransformationMatrix() const;
        glm::mat4 NormalMatrix() const;

        void SetPosition(glm::vec3 p);
        void SetScale(glm::vec3 p);
        void SetRotation(glm::vec3 p);
        
        const glm::vec3& Position() const noexcept;
        const glm::vec3& Scale() const noexcept;
        const glm::vec3& Rotation() const noexcept;

    private:
        const glm::mat4& calculateMatrix() const;
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;
        mutable glm::mat4 transformation;
        mutable glm::mat4 normal;
        mutable bool calculated = false;
    };

}

#endif //!VPSK_TRANSFORM_COMPONENT_HPP
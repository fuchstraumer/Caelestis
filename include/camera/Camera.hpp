#pragma once
#ifndef VPSK_CAMERA_HPP
#define VPSK_CAMERA_HPP
#include "vpr_stdafx.h"
#include "glm/gtc/quaternion.hpp"
namespace vpsk {

    class Camera {
    public:

        virtual ~Camera() = default;

        glm::mat4 GetProjection() const noexcept;
        glm::mat4 GetView() const noexcept;

        void SetDepthRange(const float& near, const float& far);
        void SetFOV(const float& fov_degrees);
        void SetAspectRatio(const float& aspect);

        glm::vec3 GetFrontVector() const noexcept;
        glm::vec3 GetRightVector() const noexcept;
        glm::vec3 GetUpVector() const noexcept;
        const glm::vec3& GetPosition() const noexcept;
        const glm::quat& GetRotation() const noexcept;

        void SetPosition(glm::vec3 pos);
        void SetRotation(glm::quat quat);
        void LookAt(const glm::vec3& eye_pos, const glm::vec3& look_at_pos, const glm::vec3& world_up = glm::vec3(0.0f, 1.0f, 0.0f));

        double GetNearPlane() const noexcept;
        double GetFarPlane() const noexcept;
        double GetFOV() const noexcept;

        void SetTransformation(const glm::mat4& m);

    protected:
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        float fovY{ 0.50f * glm::half_pi<float>() };
        float aspect{ 16.0f / 9.0f };
        float zNear{ 0.1f };
        float zFar{ 3000.0f };
        float transformZScale{ 1.0f };
    };

    class fpsCamera : public Camera {
    public:
        fpsCamera();
        void MouseMoveUpdate();
    };
}

#endif // !VPSK_CAMERA_HPP
#pragma once
#ifndef HEPHAESTUS_ARCBALL_CAMERA_HPP
#define HEPHAESTUS_ARCBALL_CAMERA_HPP
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtx/quaternion.hpp"

class Camera {
public:
    
    Camera() = default;
    Camera(float horizontalFov, float screen_w, float screen_h, float near_z, float far_z);

    const glm::vec3& Position() const noexcept;
    const glm::vec3& ViewDirection() const noexcept;
    const glm::mat4& ProjectionMatrix() const noexcept;
    const glm::mat4& InvProjectionMatrix() const noexcept;
    const glm::mat4& ViewMatrix() const noexcept;
    const glm::mat4& InvViewMatrix() const noexcept;
    const glm::vec2& ScreenSize() const noexcept;
    float FocalLength() const noexcept;

    void SetPosition(glm::vec3 p) noexcept;
    void LookAt(glm::vec3 target) noexcept;
    void SetRotation(glm::quat q);
    const glm::quat& Orientation() const noexcept;
    

protected:

    void updateMatrices() const;
    void updateView() const;
    virtual void updateProjection() const = 0;

    float horizontalFOV{ glm::radians(60.0f) };
    float zNear{ 0.1f };
    float zFar{ 1000.0f };
    float pivotDistance{ 10.0f };
    glm::vec2 screenSize{ 0.0f, 0.0f };
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 viewDirection{ 0.0f, 0.0f, 0.0f };
    glm::quat orientation{ 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };
    mutable glm::vec3 rightDir;
    mutable glm::vec3 upDir;
    mutable glm::vec3 negativeViewDir;
    mutable glm::mat4 projection;
    mutable glm::mat4 invProjection;
    mutable bool projCalculated{ false };
    mutable glm::mat4 view;
    mutable glm::mat4 invView;
    mutable bool viewCalculated{ false };

};

class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(float horizontalFov, float screen_w, float screen_h, float near_z, float far_z);
private:
    void updateProjection() const final;
};

#endif //!HEPHAESTUS_ARCBALL_CAMERA_HPP

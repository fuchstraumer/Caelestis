#include "camera/Camera.hpp"
#include "math/MatrixUtils.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/rotate_normalized_axis.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "imgui/imgui.h"
#include "scene/InputHandler.hpp"
namespace vpsk {

    glm::mat4 Camera::GetProjection() const noexcept {
        return glm::perspective(fovY, aspect, zNear * transformZScale, zFar * transformZScale);
    }

    glm::mat4 Camera::GetView() const noexcept {
        return glm::mat4{ glm::mat4_cast(rotation) * glm::translate(-position) };
    }

    void Camera::SetDepthRange(const float & near, const float & far) {
        zNear = near;
        zFar = far;
    }

    void Camera::SetFOV(const float & fov_degrees) {
        fovY = glm::radians(fov_degrees);
    }

    void Camera::SetAspectRatio(const float & _aspect) {
        aspect = _aspect;
    }

    glm::vec3 Camera::GetFrontVector() const noexcept {
        static const glm::vec3 z{ 0.0f, 0.0f, -1.0f };
        return glm::vec3{ glm::conjugate(rotation) * z };
    }

    glm::vec3 Camera::GetRightVector() const noexcept {
        static const glm::vec3 right{ -1.0f, 0.0f, 0.0f };
        return glm::vec3{ glm::conjugate(rotation) * right };
    }

    glm::vec3 Camera::GetUpVector() const noexcept {
        static const glm::vec3 up{ 0.0f, 1.0f, 0.0f };
        return glm::vec3{ glm::conjugate(rotation) * up };
    }

    const glm::vec3 & Camera::GetPosition() const noexcept {
        return position;
    }

    const glm::quat & Camera::GetRotation() const noexcept {
        return rotation;
    }

    void Camera::SetPosition(glm::vec3 pos) {
        position = std::move(pos);
    }

    void Camera::SetRotation(glm::quat quat) {
        rotation = std::move(quat);
    }

    glm::quat rotate_vec(glm::vec3 from, glm::vec3 to) {
        from = glm::normalize(from);
        to = glm::normalize(to);

        const float cos_angle = glm::dot(from, to);
        if (abs(cos_angle) > 0.99999f) {
            if (cos_angle > 0.99999f) {
                return glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
            }
            else {
                glm::vec3 rot = glm::cross(glm::vec3{ 1.0f,0.0f,0.0f }, from);
                if (glm::dot(rot, rot) > 0.001f) {
                    rot = glm::normalize(rot);
                }
                else {
                    rot = glm::normalize(glm::cross(glm::vec3{ 0.0f,1.0f,0.0f }, from));
                }
                return glm::quat(0.0f, rot);
            }
        }
        
        const glm::vec3 rot = glm::normalize(glm::cross(from, to));
        const glm::vec3 half_vec = glm::normalize(from + to);
        const float half_cos_range = glm::clamp(glm::dot(half_vec, from), 0.0f, 1.0f);
        const float sin_half_angle = sqrtf(1.0f - half_cos_range * half_cos_range);
        return glm::quat{ half_cos_range, rot * sin_half_angle };
    }

    glm::quat rotate_vec_axis(glm::vec3 from, glm::vec3 to, glm::vec3 axis) {
        axis = glm::normalize(axis);
        from = glm::normalize(glm::cross(axis, from));
        to = glm::normalize(glm::cross(axis, to));

        if (glm::dot(to, from) < -0.9999f) {
            return glm::quat{ 0.0f, axis };
        }

        const float quat_sign = glm::sign(glm::dot(axis, glm::cross(from, to)));
        const glm::vec3 half_vec = glm::normalize(from + to);
        const float cos_half_range = glm::clamp(glm::dot(half_vec, from), 0.0f, 1.0f);
        const float sin_half_range = quat_sign * sqrtf(1.0f - cos_half_range * cos_half_range);
        return glm::quat{ cos_half_range, axis * sin_half_range };
    }

    glm::quat look_at(const glm::vec3& dir, const glm::vec3& up) {
        static const glm::vec3 z{ 0.0f, 0.0f, 1.0f };
        static const glm::vec3 y{ 0.0f, 1.0f, 0.0f };
        const glm::vec3 norm_dir = glm::normalize(dir);
        const glm::vec3 right = glm::cross(norm_dir, up);
        const glm::vec3 actual_up = glm::cross(right, norm_dir);
        const glm::quat look_transform = rotate_vec(norm_dir, z);
        const glm::quat up_transform = rotate_vec_axis(look_transform * actual_up, y, z);
        return glm::quat{ up_transform * look_transform };
    }

    void Camera::LookAt(const glm::vec3 & eye_pos, const glm::vec3 & look_at_pos, const glm::vec3 & world_up) {
        position = eye_pos;
        rotation = look_at(look_at_pos - eye_pos, world_up);
    }

    double Camera::GetNearPlane() const noexcept {
        return static_cast<double>(zNear);
    }

    double Camera::GetFarPlane() const noexcept {
        return static_cast<double>(zFar);
    }

    double Camera::GetFOV() const noexcept {
        return static_cast<double>(fovY);
    }

    fpsCamera::fpsCamera() : Camera() {}

    void fpsCamera::MouseMoveUpdate() {
        const ImGuiIO& io = ImGui::GetIO();
        const float dx = input_handler::MouseDx;
        const float dy = input_handler::MouseDy;
        const glm::quat pitch = glm::angleAxis(dy * 0.02f, glm::vec3{ 1.0f,0.0f,0.0f });
        const glm::quat yaw = glm::angleAxis(dx * 0.02f, glm::vec3{ 0.0f,1.0f,0.0f });
        rotation = glm::normalize(pitch * rotation * yaw);
    }

}
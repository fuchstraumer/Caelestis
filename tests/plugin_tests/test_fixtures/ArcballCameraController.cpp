#include "ArcballCameraController.hpp"
#include "Camera.hpp"
#include <algorithm>
#include "imgui.h"
#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

ArcballCameraController::ArcballCameraController(Spatial * object_to_manip) : object(object_to_manip) {
    constraint.CurrentConstraintAxis = AxisSet::None;
    constraint.Nearest = 0;
    orientation.Reset = object->Orientation;
    orientation.Initial = orientation.Reset;
    orientation.Current = orientation.Reset;
}

void ArcballCameraController::Update(Camera * cam, const VkRect2D & viewport) {
    updateButtons();
    updateKeys();

    currentProjView = cam->ProjectionMatrix() * cam->ViewMatrix();

    ImGuiIO& io = ImGui::GetIO();

    drag.From = arcballCoord(viewport, mousePosInitial);
    drag.To = arcballCoord(viewport, glm::vec2(io.MousePos.x, io.MousePos.y));

    if (!dragging) {
        updateCurrentAxisSet();
        updateConstraintAxis(cam);
        constraint.Nearest = nearestConstraint(drag.To);
    }

    if (dragging) {
        updateDragArc(cam);
        object->Orientation = orientation.Current;
    }

    updateResultArc();

}

const glm::quat & ArcballCameraController::CurrentOrientation() const noexcept {
    return orientation.Current;
}

void ArcballCameraController::SetConstraintAxis(bool constrain) {
    allowConstraints = constrain;
}

void ArcballCameraController::updateButtons() {

    auto& io = ImGui::GetIO();

    //dragging = io.MouseDown[1];
    dragging = ImGui::IsMouseDragging(1);

    if (io.MouseClicked[1]) {
        mousePosInitial = glm::vec2(io.MousePos.x, io.MousePos.y);
    }
    else if (io.MouseReleased[1]) {
        orientation.Initial = orientation.Current;
    }

}

void ArcballCameraController::updateKeys() {

    auto& io = ImGui::GetIO();

    if (io.KeysDown[GLFW_KEY_KP_ADD]) {
        radius = glm::clamp(radius + 0.25f, 0.25f, 1.0f);
    }
    else if (io.KeysDown[GLFW_KEY_KP_SUBTRACT]) {
        radius = glm::clamp(radius - 0.25f, 0.25f, 1.0f);
    }

    if (io.KeysDown[GLFW_KEY_R]) {
        orientation.Initial = orientation.Reset;
        orientation.Current = orientation.Reset;
        object->Orientation = orientation.Reset;
    }

}

void ArcballCameraController::updateCurrentAxisSet() {
    
    ImGuiIO& io = ImGui::GetIO();

    const bool shift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    const bool ctrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];

    if (allowConstraints && ctrl && shift) {
        constraint.CurrentConstraintAxis = AxisSet::World;
    }
    else if (allowConstraints && ctrl) {
        constraint.CurrentConstraintAxis = AxisSet::Body;
    }
    else if (allowConstraints && shift) {
        constraint.CurrentConstraintAxis = AxisSet::Camera;
    }
    else {
        constraint.CurrentConstraintAxis = AxisSet::None;
    }

}

void ArcballCameraController::updateConstraintAxis(Camera * cam) {
    constraint.Available.clear(); constraint.Available.shrink_to_fit();

    glm::quat inv = glm::conjugate(cam->Orientation());

    switch (constraint.CurrentConstraintAxis) {
    case AxisSet::Body:
        constraint.Available = {
            inv * object->Orientation * glm::vec3(1.0f, 0.0f, 0.0f),
            inv * object->Orientation * glm::vec3(0.0f, 1.0f, 0.0f),
            inv * object->Orientation * glm::vec3(0.0f, 0.0f, 1.0f)
        };
        break;
    case AxisSet::Camera:
        constraint.Available = {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        };
        break;
    case AxisSet::World:
        constraint.Available = {
            inv * glm::vec3(1.0f, 0.0f, 0.0f),
            inv * glm::vec3(0.0f, 1.0f, 0.0f),
            inv * glm::vec3(0.0f, 0.0f, 1.0f)
        };
        break;
    case AxisSet::None:
        break;
    default:
        throw std::domain_error("Invalid axis set enum value for current constraint type!");
    }
}

void ArcballCameraController::updateDragArc(Camera * cam) {

    if (constraint.CurrentConstraintAxis != AxisSet::None) {
        drag.From = constrainToAxis(drag.From, constraint.Available[constraint.Nearest]);
        drag.To = constrainToAxis(drag.To, constraint.Available[constraint.Nearest]);
    }

    float w = glm::dot(drag.From, drag.To);
    glm::vec3 v = cam->Orientation() * glm::cross(drag.From, drag.To);
    glm::quat orientation_drag(w, v.x, v.y, v.z);

    orientation.Current = glm::normalize(orientation_drag * orientation.Initial);

}

void ArcballCameraController::updateResultArc() {

    const glm::quat q = orientation.Initial;
    float s = sqrtf(q.x * q.x + q.y * q.y);
    if (s == 0.0f) {
        result.From.x = 0.0f;
        result.From.y = 1.0f;
        result.From.z = 0.0f;
    }
    else {
        result.From.x = -q.y / s;
        result.From.y = q.x / s;
        result.From.z = 0.0f;
    }

    result.To.x = q.w * result.From.x - q.z * result.From.y;
    result.To.y = q.w * result.From.y + q.z * result.From.x;
    result.To.z = q.x * result.From.y - q.y * result.From.x;

    if (q.w < 0.0f) {
        result.From.x = -result.From.x;
        result.From.y = -result.From.y;
        result.From.z = 0.0f;
    }

}

glm::vec3 ArcballCameraController::arcballCoord(const VkRect2D & viewport, glm::vec2 mouse_pos) {
    glm::vec3 windowMousePos = windowCoord(viewport, mouse_pos);
    glm::vec3 center = glm::vec3(0.0f);
    glm::vec3 point = (windowMousePos - center) / radius;

    float r = glm::length2(point);
    if (r > 1.0f) {
        point *= (1.0f / sqrtf(r));
    }
    else {
        point.z = sqrtf(1.0f - r);
    }

    return point;
}

glm::vec3 ArcballCameraController::windowCoord(const VkRect2D & viewport, glm::vec2 pos) {
    return glm::vec3{
       -(2.0f * (pos.x - static_cast<float>(viewport.offset.x)) / static_cast<float>(viewport.extent.width) - 1.0f),
       -(2.0f * (pos.y - static_cast<float>(viewport.offset.y)) / static_cast<float>(viewport.extent.height) - 1.0f),
        0.0f
    };
}

glm::vec3 ArcballCameraController::constrainToAxis(glm::vec3 point, glm::vec3 axis) {
    glm::vec3 point_on_plane;
    glm::vec3 proj = point - (axis * glm::dot(axis, point));

    float length = glm::length(proj);
    if (length > 0.0f) {
        float s = 1.0f / length;
        if (proj.z < 0.0f) {
            s = -s;
        }
        point_on_plane = proj * s;
    }
    else if (axis.z == 1.0f) {
        point_on_plane = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    else {
        point_on_plane = glm::normalize(glm::vec3(-axis.y, axis.x, 0.0f));
    }

    return point_on_plane;
}

int ArcballCameraController::nearestConstraint(glm::vec3 ball_point) {
    float max = -std::numeric_limits<float>::infinity();
    int nearest = 0;
    for (size_t i = 0; i < constraint.Available.size(); ++i) {
        glm::vec3 point_on_plane = constrainToAxis(ball_point, constraint.Available[i]);
        float dot = glm::dot(point_on_plane, ball_point);
        if (dot > max) {
            max = dot;
            nearest = static_cast<int>(i);
        }
    }

    return nearest;
}


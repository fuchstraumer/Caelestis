#pragma once
#ifndef ARCBALL_CAMERA_CONTROLLER_HPP
#define ARCBALL_CAMERA_CONTROLLER_HPP
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>

class Camera;

struct Spatial {
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::quat Orientation;
    glm::mat4 LocalTransform;
    glm::mat4 WorldTransform;
};

enum class AxisSet : uint8_t {
    None = 0,
    Camera = 1,
    Body = 2,
    World = 3
};

struct arc_t {
    glm::vec3 From = glm::vec3(0.0f);
    glm::vec3 To = glm::vec3(0.0f);
};

struct Constraint {
    AxisSet CurrentConstraintAxis;
    std::vector<glm::vec3> Available;
    uint32_t Nearest;
};

struct Orientation {
    // Reset quaternion might not be a simple (0,0,0) sort of deal
    glm::quat Reset = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::quat Initial = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::quat Current = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}; 

class ArcballCameraController {
public:

    ArcballCameraController() = default;
    ArcballCameraController(Spatial* object_to_manip);

    void Update(Camera* cam, const struct VkRect2D& viewport);
    const glm::quat& CurrentOrientation() const noexcept;
    void SetConstraintAxis(bool constrain);

private:

    friend class ArcballHelper;

    void updateButtons();
    void updateKeys();
    void updateCurrentAxisSet();
    void updateConstraintAxis(Camera* cam);
    void updateDragArc(Camera* cam);
    void updateResultArc();

    glm::vec3 arcballCoord(const struct VkRect2D& viewport, glm::vec2 mouse_pos);
    glm::vec3 windowCoord(const struct VkRect2D& viewport, glm::vec2 pos);
    glm::vec3 constrainToAxis(glm::vec3 point, glm::vec3 axis);
    int nearestConstraint(glm::vec3 ball_point);

    Spatial* object;
    glm::mat4 currentProjView;
    bool allowConstraints{ true };
    float radius = 0.5f;
    bool dragging{ false };
    glm::vec2 mousePosInitial{ 0.0f };
    arc_t drag;
    arc_t result;
    Orientation orientation;
    Constraint constraint;

};

#endif //!ARCBALL_CAMERA_CONTROLLER_HPP

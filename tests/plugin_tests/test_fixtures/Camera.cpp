#include "Camera.hpp"
#include "glm/glm.hpp"
#include "glm/vector_relational.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/matrix_transform.hpp"

glm::mat4 alignZAxisWithTarget( glm::vec3 targetDir, glm::vec3 upDir ) {
    using namespace glm;

    // Ensure that the target direction is non-zero.
    if (length2(targetDir) == 0)
    targetDir = vec3(0, 0, 1);

    // Ensure that the up direction is non-zero.
    if (length2(upDir) == 0)
    upDir = vec3(0, 1, 0);

    // Check for degeneracies.  If the upDir and targetDir are parallel 
    // or opposite, then compute a new, arbitrary up direction that is
    // not parallel or opposite to the targetDir.
    if (length2(cross(upDir, targetDir)) == 0) {
        upDir = cross(targetDir, vec3(1, 0, 0));
        if (length2(upDir) == 0)
            upDir = cross(targetDir, vec3(0, 0, 1));
    }

    // Compute the x-, y-, and z-axis vectors of the new coordinate system.
    vec3 targetPerpDir = cross(upDir, targetDir);
    vec3 targetUpDir = cross(targetDir, targetPerpDir);

    // Rotate the x-axis into targetPerpDir (row 0),
    // rotate the y-axis into targetUpDir   (row 1),
    // rotate the z-axis into targetDir     (row 2).
    vec3 row[3];
    row[0] = normalize(targetPerpDir);
    row[1] = normalize(targetUpDir);
    row[2] = normalize(targetDir);

    return glm::mat4{ 
        row[0].x,  row[0].y,  row[0].z,  0.0f,
        row[1].x,  row[1].y,  row[1].z,  0.0f,
        row[2].x,  row[2].y,  row[2].z,  0.0f,
        0.0f,      0.0f,      0.0f,	     1.0f 
    };
}

Camera::Camera(float horizontalFov, float screen_w, float screen_h, float near_z, float far_z) : horizontalFOV(horizontalFov), screenSize(screen_w, screen_h), zNear(near_z), zFar(far_z), position(-2.0f, 2.0f, 1.0f) {
    viewDirection = glm::normalize(glm::vec3(0.0f) - position);
    view = glm::lookAt(position, viewDirection, worldUp);
    upDir = worldUp;
}

const glm::vec3& Camera::Position() const noexcept {
    return position;
}

const glm::vec3 & Camera::ViewDirection() const noexcept {
    return viewDirection;
}

const glm::mat4 & Camera::ProjectionMatrix() const noexcept {
    if (!projCalculated) {
        updateProjection();
    }
    return projection;
}

const glm::mat4 & Camera::InvProjectionMatrix() const noexcept {
    if (!projCalculated) {
        updateProjection();
    }
    return invProjection;
}

const glm::mat4 & Camera::ViewMatrix() const noexcept {
    if (!viewCalculated) {
        updateView();
    }
    return view;
}

const glm::mat4 & Camera::InvViewMatrix() const noexcept {
    if (!viewCalculated) {
        updateView();
    }
    return invView;
}

const glm::vec2 & Camera::ScreenSize() const noexcept {
    return screenSize;
}

float Camera::FocalLength() const noexcept {
    return 1.0f / (std::tanf(glm::radians(horizontalFOV) * 0.50f) * 2.0f);
}

void Camera::SetPosition(glm::vec3 p) noexcept {
    position = p;
}

void Camera::LookAt(glm::vec3 target) noexcept {
    viewDirection = glm::normalize(target - position);
    //glm::vec3 axis = glm::cross(viewDirection, upDir);
    orientation = glm::toQuat(alignZAxisWithTarget(viewDirection, worldUp));
    pivotDistance = glm::distance(target, position);
    viewCalculated = false;
}

void Camera::SetRotation(glm::quat q) {
    orientation = q;
    viewCalculated = false;
}

const glm::quat & Camera::Orientation() const noexcept {
    return orientation;
}

void Camera::updateMatrices() const {

    if (!viewCalculated) {
        updateView();
    }

    if (!projCalculated) {
        updateProjection();
    }

}

void Camera::updateView() const {
    rightDir = -glm::normalize(viewDirection);
    upDir = glm::rotate(orientation, glm::vec3(1.0f, 0.0f, 0.0f));
    negativeViewDir = glm::rotate(orientation, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 d(-glm::dot(position, upDir), -glm::dot(position, negativeViewDir), -glm::dot(position, rightDir));
    view[0][0] = upDir.x; view[1][0] = upDir.y; view[2][0] = upDir.z; view[3][0] = d.x;
    view[0][1] = negativeViewDir.x; view[1][1] = negativeViewDir.y; view[2][1] = negativeViewDir.z; view[3][1] = d.y;
    view[0][2] = rightDir.x; view[1][2] = rightDir.y; view[2][2] = rightDir.z; view[3][2] = d.z;
    view[0][3] = 0.0f; view[1][3] = 0.0f; view[2][3] = 0.0f; view[3][3] = 1.0f;

    invView = glm::inverse(view);
    viewCalculated = true;
}

PerspectiveCamera::PerspectiveCamera(float horizontalFov, float screen_w, float screen_h, float near_z, float far_z) : Camera(horizontalFov, screen_w, screen_h, near_z, far_z) {}

void PerspectiveCamera::updateProjection() const {
    projection = glm::perspectiveFov(glm::radians(horizontalFOV), screenSize.x, screenSize.y, zNear, zFar);
    projection[1][1] *= -1.0f;
}

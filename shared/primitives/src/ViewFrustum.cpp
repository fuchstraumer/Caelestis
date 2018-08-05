#include "../include/ViewFrustum.hpp"

ViewFrustum::ViewFrustum() : view(1.0f) {
    planes.fill(glm::vec4(0.0f));
    planeNormals.fill(glm::vec4(0.0f));
}

ViewFrustum::ViewFrustum(const float screen_depth, const glm::mat4 & projection, const glm::mat4 & _view, const glm::mat4 world) {
    Construct(screen_depth, projection, _view, world)
}

void ViewFrustum::Construct(const float screen_depth, const glm::mat4 & projection, const glm::mat4 & _view, const glm::mat4 world) {
    view = _view;
    glm::mat4 proj = projection;

    const float z_min = -projection[3][2] / projection[2][2];
    const float r = screen_depth / (screen_depth - z_min);
}

bool ViewFrustum::ContainsPoint(const glm::vec3 & p) const {
    return false;
}

bool ViewFrustum::ContainsSphere(const glm::vec3 & p, const float & radius) {
    return false;
}

bool ViewFrustum::ContainsAABB(const AABB & box) {
    return false;
}

bool ViewFrustum::IntersectsAABB(const AABB & box) {
    return false;
}

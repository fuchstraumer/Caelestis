#include "components/graphics/TransformComponent.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat3x3.hpp"
namespace vpsk {

    WorldTransformComponent::WorldTransformComponent(glm::vec3 p, glm::vec3 s, glm::vec3 rot)  :
            position(std::move(p)), scale(std::move(s)), rotation(std::move(rot)) {
        calculateMatrix();
    }

    const glm::mat4& WorldTransformComponent::operator()() const {
        return TransformationMatrix();
    }

    const glm::mat4& WorldTransformComponent::TransformationMatrix() const {
        if (!calculated) {
            return calculateMatrix();
        }
        else {
            return transformation;
        }
    }

    glm::mat4 WorldTransformComponent::NormalMatrix() const {
        if (!calculated) {
            calculateMatrix();
            return normal;
        }
        else {
            return normal;
        }
    }

    void WorldTransformComponent::SetPosition(glm::vec3 p) {
        position = std::move(p);
        calculated = false;
    }

    void WorldTransformComponent::SetScale(glm::vec3 s) {
        scale = std::move(s);
        calculated = false;
    }

    void WorldTransformComponent::SetRotation(glm::vec3 s) {
        rotation = std::move(s);
        calculated = false;
    }

    const glm::vec3& WorldTransformComponent::Position() const noexcept {
        return position;
    }

    const glm::vec3& WorldTransformComponent::Scale() const noexcept {
        return scale;
    }

    const glm::vec3& WorldTransformComponent::Rotation() const noexcept {
        return rotation;
    }

    const glm::mat4& WorldTransformComponent::calculateMatrix() const {
        if (calculated) {
            return transformation;
        }
        else {
            glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);
            glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 rotation_x_matrix = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 rotation_y_matrix = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotation_z_matrix = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            transformation =  translation_matrix * rotation_x_matrix * rotation_y_matrix * rotation_z_matrix * scale_matrix;
            normal = glm::transpose(glm::inverse(glm::mat3(transformation)));
            calculated = true;
            return transformation;
        }
    }

}
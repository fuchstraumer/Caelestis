#include "geometries/TriangleMesh.hpp"

using namespace vpr;

namespace vpsk {

    TriangleMesh::TriangleMesh(const glm::vec3& _position, const glm::vec3& _scale, const glm::vec3& _rotation) : position(_position), scale(_scale), rotation(_rotation) {}

    TriangleMesh::~TriangleMesh() {
        DestroyVulkanObjects();
    }

    uint32_t TriangleMesh::AddVertex(vertex_t vertex) noexcept {
        vertexPositions.push_back(std::move(vertex.pos));
        vertexData.push_back(std::move(vertex_data_t{ vertex.normal, vertex.tangent, vertex.uv }));
        return static_cast<uint32_t>(vertexPositions.size() - 1);
    }

    void TriangleMesh::AddIndex(uint32_t index) noexcept {
        indices.push_back(std::move(index));
    }

    void TriangleMesh::AddTriangle(uint32_t i0, uint32_t i1, uint32_t i2) noexcept {
        indices.insert(indices.cend(), { std::move(i0), std::move(i1), std::move(i2) });
    }

    const glm::vec3& TriangleMesh::GetVertexPosition(const uint32_t& idx) const noexcept {
        return vertexPositions[idx];
    }

    const TriangleMesh::vertex_data_t& TriangleMesh::GetVertexData(const uint32_t& idx) const noexcept {
        return vertexData[idx];
    }

    vertex_t TriangleMesh::GetVertex(const uint32_t& index) const {
        return vertex_t{ vertexPositions[index], vertexData[index].normal, vertexData[index].tangent, vertexData[index].uv };
    }

    void TriangleMesh::ReserveVertices(const size_t& reserve_amount) noexcept {
        vertexPositions.reserve(reserve_amount);
        vertexData.reserve(reserve_amount);
    }

    void TriangleMesh::ReserveIndices(const size_t& reserve_amount) noexcept {
        indices.reserve(reserve_amount);
    }

    size_t TriangleMesh::NumVertices() const noexcept {
        if(!vertexPositions.empty() && !vertexData.empty()) {
            assert(vertexPositions.size() == vertexData.size());
            return vertexPositions.size();
        }
        else {
            return numVertices;
        }
    }

    size_t TriangleMesh::NumIndices() const noexcept {
        if(!indices.empty()) {
            return indices.size();
        }
        else {
            return numIndices;
        }
    }

    void TriangleMesh::CreateBuffers(const Device* dvc) {
        
        device = dvc;

        // Set these now, as they are "frozen" and might be zero if set after this point.
        numVertices = static_cast<uint32_t>(vertexPositions.size());
        numIndices = static_cast<uint32_t>(indices.size());

        vbo0 = std::make_unique<Buffer>(device);
        vbo1 = std::make_unique<Buffer>(device);
        ebo = std::make_unique<Buffer>(device);

        vbo0->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::vec3) * NumVertices());
        vbo1->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(vertex_data_t) * NumVertices());
        ebo->CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * NumIndices());
        
    }

    void TriangleMesh::RecordTransferCommands(const VkCommandBuffer& transfer_cmd) {
        
        vbo0->CopyTo(vertexPositions.data(), transfer_cmd, sizeof(glm::vec3) * NumVertices(), 0);
        vbo1->CopyTo(vertexData.data(), transfer_cmd, sizeof(vertex_data_t) * NumVertices(), 0);
        ebo->CopyTo(indices.data(), transfer_cmd, sizeof(uint32_t) * NumIndices(), 0);
    
    }

    void TriangleMesh::Render(const VkCommandBuffer& draw_cmd) const noexcept {
    
        bindBuffers(draw_cmd);
        drawIndexed(draw_cmd);

    }

    void TriangleMesh::DestroyVulkanObjects() {
        
        ebo.reset();
        vbo0.reset();
        vbo1.reset();

    }

    void TriangleMesh::FreeCpuData() {

        vertexPositions.clear();
        vertexPositions.shrink_to_fit();
        vertexData.clear();
        vertexData.shrink_to_fit();
        indices.clear();
        indices.shrink_to_fit();

    }

    const glm::mat4& TriangleMesh::GetModelMatrix() const noexcept {
        if (!modelMatrixCached) {
            updateModelMatrix();
        }
        return model;
    }

    void TriangleMesh::SetModelMatrix(const glm::mat4& updated_model) {
        model = updated_model;
    }

    void TriangleMesh::UpdatePosition(const glm::vec3& new_position) {
        position = new_position;
        modelMatrixCached = false;
    }

    void TriangleMesh::UpdateScale(const glm::vec3& new_scale) {
        scale = new_scale;
        modelMatrixCached = false;
    }

    void TriangleMesh::UpdateRotation(const glm::vec3& new_rotation) {
        rotation = new_rotation;
        modelMatrixCached = false;
    }

    const glm::vec3& TriangleMesh::GetPosition() const noexcept {
        return position;
    }

    const glm::vec3& TriangleMesh::GetScale() const noexcept {
        return scale;
    }

    const glm::vec3& TriangleMesh::GetRotation() const noexcept {
        return rotation;
    }

    void TriangleMesh::updateModelMatrix() const {
        glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotation_x_matrix = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotation_y_matrix = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotation_z_matrix = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = translation_matrix * rotation_x_matrix * rotation_y_matrix * rotation_z_matrix * scale_matrix;
        modelMatrixCached = true;
    }

    void TriangleMesh::bindBuffers(const VkCommandBuffer& draw_cmd) const noexcept {
        constexpr static VkDeviceSize offsets[2] { 0, 0 };
        VkBuffer handles[2]{ vbo0->vkHandle(), vbo1->vkHandle() };
        vkCmdBindVertexBuffers(draw_cmd, 0, 2, handles, offsets);
        vkCmdBindIndexBuffer(draw_cmd, ebo->vkHandle(), 0, VK_INDEX_TYPE_UINT32);
    }

    void TriangleMesh::drawIndexed(const VkCommandBuffer& draw_cmd_buffer) const noexcept {
        vkCmdDrawIndexed(draw_cmd_buffer, static_cast<uint32_t>(NumIndices()), 1, 0, 0, 0);
    }
}
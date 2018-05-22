#pragma once
#ifndef VPSK_MESH_DATA_HPP
#define VPSK_MESH_DATA_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "components/graphics/MeshBufferComponents.hpp"

namespace vpsk {

    struct vertex_data_t {
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec2 UV;
    };

    /**All mesh objects should use this mesh data format. They don't have to bind or use
     * all fields at drawtime, but this helps standardize things considerably. 
     */
    struct MeshData {

        MeshData() noexcept;
        ~MeshData();
        MeshData(MeshData&& other) noexcept;
        MeshData& operator=(MeshData&& other) noexcept;

        explicit operator VertexBufferComponent() const;
        explicit operator IndexBufferComponent() const;

        std::vector<glm::vec3> Positions;
        std::vector<vertex_data_t> Vertices;
        std::vector<uint32_t> Indices;

        std::unique_ptr<vpr::Buffer> VBO0;
        std::unique_ptr<vpr::Buffer> VBO1;
        std::unique_ptr<vpr::Buffer> EBO;

        void CreateBuffers(const vpr::Device* device);
        void TransferToDevice(VkCommandBuffer cmd);
        void FreeStagingBuffers();

        std::vector<VkBuffer> GetVertexBuffers() const;

    private:

        std::unique_ptr<vpr::Buffer> vboStaging0;
        std::unique_ptr<vpr::Buffer> vboStaging1;
        std::unique_ptr<vpr::Buffer> eboStaging;
    };

}

#endif //!VPSK_MESH_DATA_HPP

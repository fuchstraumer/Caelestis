#pragma once
#ifndef ASSET_PIPELINE_MESH_DATA_HPP
#define ASSET_PIPELINE_MESH_DATA_HPP
#include <cstdint>

struct MeshProcessingOptions {
    bool Interleaved;
    bool Use_fp16;
    bool AllowInt16_Indices;
};

struct MeshDataHeader {
    char Magic[12]{ "Hephaestus\0" };
    uint32_t Version{ 0x00000000 };
    float Center[3]{ 0.0f, 0.0f, 0.0f };
    float HalfExtent[3]{ 0.0f, 0.0f, 0.0f };
    uint32_t NumParts{ 0 };
    uint32_t NumMaterials{ 0 };
    uint32_t Interleaved{ 0 };
    uint32_t PositionAttrOffset{ 0xffffffff };
    uint32_t PositionAttrStride{ 0xffffffff };
    uint32_t PositionAttrFormat{ 0xffffffff };
    uint32_t TangentAttrOffset{ 0xffffffff };
    uint32_t TangentAttrStride{ 0xffffffff };
    uint32_t TangentAttrFormat{ 0xffffffff };
    uint32_t UV0_Offset{ 0xffffffff };
    uint32_t UV0_Stride{ 0xffffffff };
    uint32_t UV0_Format{ 0xffffffff };
    uint32_t UV1_Offset{ 0xffffffff };
    uint32_t UV1_Stride{ 0xffffffff };
    uint32_t UV1_Format{ 0xffffffff };
    uint32_t VertexCount{ 0xffffffff };
    uint32_t VertexDataSize{ 0xffffffff };
    uint32_t IndexFormat{ 0xffffffff };
    uint32_t IndexCount{ 0xffffffff };
    uint32_t IndexDataSize{ 0xffffffff };
};

struct fvec2 {
    constexpr fvec2() noexcept : data{ 0.0f } {}

    constexpr fvec2(float v0, float v1) noexcept : data{ v0, v1 } {}

    float& operator[](const size_t idx) {
        return data[idx];
    }

    const float& operator[](const size_t idx) const {
        return data[idx];
    }

    constexpr const float& x() const noexcept {
        return data[0];
    }

    constexpr const float& y() const noexcept {
        return data[1];
    }

private:
    float data[2];
};

struct fvec4 {
    constexpr fvec4() noexcept : data{ 0.0f } {}
    constexpr fvec4(float v0, float v1, float v2, float v3) noexcept :
        data{ v0, v1, v2, v3 } {}

    constexpr float& operator[](const size_t idx) {
        return data[idx];
    }

    constexpr const float& operator[](const size_t idx) const {
        return data[idx];
    }

    constexpr const float& x() const noexcept {
        return data[0];
    }

    constexpr const float& y() const noexcept {
        return data[1];
    }
    
    constexpr const float& z() const noexcept {
        return data[2];
    }

    constexpr const float& w() const noexcept {
        return data[3];
    }

private:
    float data[4];
};

struct Vertex {
    // data using this type is encoded as 
    // half-precision floats
    fvec4 position;
    int16_t tangents[4];
    fvec2 uv0;
};

struct UninterleavedVertexData {
    fvec4* Positions{ nullptr };
    int16_t* Tangents{ nullptr };
    uint8_t* Colors{ nullptr };
    fvec2* UV0s{ nullptr };
    fvec2* UV1s{ nullptr };
};

struct PartData {
    uint32_t IndexOffset{ 0xffffffff };
    uint32_t IndexCount{ 0xffffffff };
    uint32_t MinIndex{ 0xffffffff };
    uint32_t MaxIndex{ 0xffffffff };
    uint32_t MaterialID{ 0xffffffff };
    float Center[3]{ 0.0f, 0.0f, 0.0f };
    float HalfExtent[3]{ 0.0f, 0.0f, 0.0f };
};

struct MaterialInfo {
    uint32_t NameLength;
    char* Name;
};

struct MeshData {
    static void DestroyMeshData(MeshData* data);
    MeshDataHeader Header{ };
    PartData* Parts{ nullptr };
    MaterialInfo* Materials{ nullptr };
    // Either an array of Vertex structures, or a poitner to "UninterleavedVertexData"
    void* Vertices{ nullptr };
    void* Indices{ nullptr };
};

#endif //!ASSET_PIPELINE_MESH_DATA_HPP

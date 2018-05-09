#include "systems/gltfLoader.hpp"
#include <experimental/filesystem>
#include <vulkan/vulkan.h>
#include "fx-gltf/include/fx/gltf.h"
namespace vpsk {

    static std::unordered_map<std::string, fx::gltf::Document>& GetProgramDocuments() noexcept {
        static std::unordered_map<std::string, fx::gltf::Document> map;
        return map;
    }

    static std::unordered_map<std::string, mesh_data_t>& GetMeshData() noexcept {
        static std::unordered_map<std::string, mesh_data_t> meshes;
        return meshes;
    }

    static uint32_t CalculateDataTypeSize(fx::gltf::Accessor const & accessor) noexcept {

        uint32_t elementSize = 0;

        switch (accessor.componentType) {
        case fx::gltf::Accessor::ComponentType::Byte:
        case fx::gltf::Accessor::ComponentType::UnsignedByte:
            elementSize = 1;
            break;
        case fx::gltf::Accessor::ComponentType::Short:
        case fx::gltf::Accessor::ComponentType::UnsignedShort:
            elementSize = 2;
            break;
        case fx::gltf::Accessor::ComponentType::Float:
        case fx::gltf::Accessor::ComponentType::UnsignedInt:
            elementSize = 4;
            break;
        }

        switch (accessor.type) {
        case fx::gltf::Accessor::Type::Mat2:
            return 4 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Mat3:
            return 9 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Mat4:
            return 16 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Scalar:
            return elementSize;
            break;
        case fx::gltf::Accessor::Type::Vec2:
            return 2 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Vec3:
            return 3 * elementSize;
            break;
        case fx::gltf::Accessor::Type::Vec4:
            return 4 * elementSize;
            break;
        }

        return 0;
    }

    static VkFilter GetFilterMode(uint16_t gltf_enum) {
        switch (gltf_enum) {
        case 9728:
            return VK_FILTER_NEAREST;
        case 9729:
            return VK_FILTER_LINEAR;
        default:
            return VkFilter(0);
        }
    }

    static VkSamplerAddressMode GetMipMapFilterMode(uint16_t gltf_enum) {
        switch (gltf_enum) {
        case 33071:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case 33648:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case 10497:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        default:
            throw std::domain_error("Invalid gltf value for sampler address mode.");
        }
    }

    gltfLoader::gltfLoader(const vpr::Device * dvc, const char * fname) : device(dvc) {
        loadFromFile(fname);
    }

    void gltfLoader::setDocumentPtr(const char * fname) {
        namespace fs = std::experimental::filesystem;
        fs::path document_path = fs::absolute(fs::path(fname));

        if (!fs::exists(document_path)) {
            throw std::runtime_error("Invalid gltf file path!");
        }

        bool is_binary = false;
        const std::string extension = document_path.extension().string();
        if (extension == ".glb") {
            is_binary = true;
        }

        auto doc_map = GetProgramDocuments();
        const std::string path_str = document_path.string();

        if (doc_map.count(path_str) != 0) {
            document = &doc_map.at(path_str);
        }
        else {
            if (is_binary) {
                doc_map.emplace(path_str, fx::gltf::LoadFromBinary(path_str));
            }
            else {
                doc_map.emplace(path_str, fx::gltf::LoadFromText(path_str));
            }
            document = &doc_map.at(path_str);
        }
    }

    void gltfLoader::loadFromFile(const char* fname) {
        setDocumentPtr(fname);

    }

}

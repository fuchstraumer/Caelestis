#include "systems/gltfData.hpp"
#include <experimental/filesystem>
#include <vulkan/vulkan.h>
#include "fx-gltf/include/fx/gltf.h"
#include <mutex>

namespace vpsk {

    static std::unordered_map<std::string, fx::gltf::Document>& GetProgramDocuments() noexcept {
        static std::unordered_map<std::string, fx::gltf::Document> map;
        return map;
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

    void gltfData::Load(const LoadRequest & request) {
        namespace fs = std::experimental::filesystem;
        FilePath = fs::absolute(fs::path(request.AbsolutePath)).string();

        if (!fs::exists(FilePath)) {
            throw std::runtime_error("Invalid gltf file path!");
        }
        
        FileName = fs::path(request.AbsolutePath).filename().string();

        const std::string extension = fs::path(request.AbsolutePath).extension().string();
        if (extension == ".glb") {
            binaryFile = true;
        }

        loadFromFile();
    }

    void gltfData::loadFromFile() {
        static std::mutex documentMutex;

        auto doc_map = GetProgramDocuments();

        if (doc_map.count(FilePath) != 0) {
            document = &doc_map.at(FilePath);
        }
        else {
            if (binaryFile) {
                std::lock_guard<std::mutex> guard(documentMutex);
                doc_map.emplace(FilePath, fx::gltf::LoadFromBinary(FilePath));
            }
            else {
                std::lock_guard<std::mutex> guard(documentMutex);
                doc_map.emplace(FilePath, fx::gltf::LoadFromText(FilePath));
            }
            document = &doc_map.at(FilePath);
        }
    }

    void gltfData::parseDocument() {

    }

}

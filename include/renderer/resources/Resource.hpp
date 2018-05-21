#pragma once
#ifndef VPSK_RESOURCE_WRAPPER_HPP
#define VPSK_RESOURCE_WRAPPER_HPP
#include <memory>
#include <string>

namespace vpsk {

    class Resource {
        friend class ResourcePool;
    public:

        enum class Type : uint8_t {
            Scene,
            SceneNode,
            Material,
            Mesh,
            Light,
            Texture,
            Pipeline,
            Shader,
            ShaderGroup,
            ShaderPack,
            Camera,
            Primitive, // Skybox, box, etc. One of the built-in types
            UserType, // User-defined type
            Undefined // Invalid type
        };

        struct Handle {
            union {
                struct {
                    uint32_t type : 8;
                    uint32_t index : 24;
                };
                uint32_t Value;
            };

            explicit operator uint32_t() noexcept {
                return Value;
            }

            bool operator==(const Handle& other) const noexcept {
                return Value == other.Value;
            }
        };

        Resource(const Type resource_type, const std::string& file_path);
        Resource(const Resource&) = delete;
        Resource(Resource&&) = delete;
        Resource& operator=(const Resource&) = delete;
        Resource& operator=(Resource&&) = delete;
        virtual ~Resource() = default;

        Type GetType() const noexcept;
        Handle GetHandle() const noexcept;
        const std::string& GetName() const noexcept;
        void SetName(std::string name);

    protected:
        std::string absoluteFilePath;
        std::string name;
        Handle handle;
    };

}

#endif //!VPSK_RESOURCE_WRAPPER_HPP
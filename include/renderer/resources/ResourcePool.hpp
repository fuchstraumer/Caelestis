#pragma once
#ifndef VPSK_RESOURCE_POOL_HPP
#define VPSK_RESOURCE_POOL_HPP
#include <string>
#include <memory>
#include <unordered_map>
namespace vpsk {

    class MeshData;
    class Texture;

    class ResourcePool {
        ResourcePool(const ResourcePool&) = delete;
        ResourcePool& operator=(const ResourcePool&) = delete;
    public:
        ResourcePool() noexcept = default;

        static ResourcePool& GetResourcePool() noexcept;

        MeshData* FindMeshData(const std::string& name);
        Texture* FindTexture(const std::string& name);

        
    private:
        std::unordered_map<std::string, std::unique_ptr<MeshData>> meshes;
        std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
    };
    

}

#endif //!VPSK_RESOURCE_POOL_HPP
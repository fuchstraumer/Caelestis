#pragma once
#ifndef VPSK_IMAGE_RESOURCE_CACHE_HPP
#define VPSK_IMAGE_RESOURCE_CACHE_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace st {
    class ResourceUsage;
    class ShaderResource;
    class ShaderGroup;
}

namespace vpsk {

    class Texture;

    class ImageResourceCache {
        ImageResourceCache(const ImageResourceCache&) = delete;
        ImageResourceCache& operator=(const ImageResourceCache&) = delete;
    public:

        ImageResourceCache(const vpr::Device* dvc);
        ~ImageResourceCache();

        void AddResources(const std::vector<st::ShaderResource*>& resources);
        void AddResource(const st::ShaderResource* rsrc);

        vpsk::Texture* FindTexture(const std::string& name);
        vpr::Image* FindImage(const std::string& name);
        vpr::Sampler* FindSampler(const std::string& name);
        vpsk::Texture* Texture(const std::string& name);
        vpr::Image* Image(const std::string& name);
        vpr::Sampler* Sampler(const std::string& name);

    private:
        std::unordered_map<std::string, std::unique_ptr<vpr::Sampler>> samplers;
        std::unordered_map<std::string, std::unique_ptr<vpsk::Texture>> textures;
        std::unordered_map<std::string, std::unique_ptr<vpr::Image>> images;
    };

}

#endif //!VPSK_IMAGE_RESOURCE_CACHE_HPP

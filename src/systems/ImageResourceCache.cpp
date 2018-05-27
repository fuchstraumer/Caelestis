#include "systems/ImageResourceCache.hpp"
#include "resource/Image.hpp"
#include "resource/Sampler.hpp"
#include "resources/Texture.hpp"

namespace vpsk {

    ImageResourceCache::ImageResourceCache(const vpr::Device * dvc) : device(dvc) {}

    ImageResourceCache::~ImageResourceCache() { }

    vpr::Image* ImageResourceCache::CreateImage(const std::string& name, const VkImageCreateInfo& img_info, const VkImageViewCreateInfo& view_info) {
        auto emplaced  = images.emplace(name, std::make_unique<vpr::Image>(device));
        if (!emplaced.second) {
            throw std::runtime_error("Failed to create image in ImageResourceCache");
        }
        emplaced.first->second->Create(img_info);
        emplaced.first->second->CreateView(view_info);
        return emplaced.first->second.get();
    }

    vpr::Sampler* ImageResourceCache::CreateSampler(const std::string& name, const VkSamplerCreateInfo& sampler_info) {
        auto emplaced = samplers.emplace(name, std::make_unique<vpr::Sampler>(device));
        if (!emplaced.second) {
            throw std::runtime_error("Failed to create sampler in ImageResourceCache");
        }
        return emplaced.first->second.get();
    }

    void ImageResourceCache::AddResources(const std::vector<st::ShaderResource*>& resources) {
    }

    void ImageResourceCache::AddResource(const st::ShaderResource * rsrc) {
    }

    vpsk::Texture* ImageResourceCache::FindTexture(const std::string& name) {
        auto iter = textures.find(name);
        if (iter == textures.end()) {
            return nullptr;
        }
        else {
            return iter->second.get();
        }
    }

    vpr::Image* ImageResourceCache::FindImage(const std::string& name) {
        auto iter = images.find(name);
        if (iter == images.end()) {
            return nullptr;
        }
        else {
            return iter->second.get();
        }
    }

    vpr::Sampler* ImageResourceCache::FindSampler(const std::string& name) {
        auto iter = samplers.find(name);
        if (iter == samplers.end()) {
            return nullptr;
        }
        else {
            return iter->second.get();
        }
    }

    vpsk::Texture* ImageResourceCache::Texture(const std::string& name) {
        return textures.at(name).get();
    }

    vpr::Image* ImageResourceCache::Image(const std::string& name) {
        return images.at(name).get();
    }

    vpr::Sampler* ImageResourceCache::Sampler(const std::string& name) {
        return samplers.at(name).get();
    }

}

#include "systems/ResourceLoader.hpp"

namespace vpsk {

    void ResourceLoader::Add(const LoadRequest & item)
    {
    }

    ResourceDataHandle & ResourceLoader::GetResource(const std::string & absolute_file_path) {
        if (data.count(absolute_file_path) == 0) {
            return std::weak_ptr<ResourceData>();
        }
        else {
            return std::weak_ptr<ResourceData>(data.at(absolute_file_path));
        }
    }

    ResourceLoader & ResourceLoader::GetResourceLoader() noexcept {
        static ResourceLoader loader;
        return loader;
    }

}

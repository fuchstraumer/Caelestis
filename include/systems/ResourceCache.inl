#include "ResourceCache.hpp"
namespace vpsk {

    template<typename ResourceType>
    inline size_type ResourceCache<ResourceType>::Size() const noexcept {
        return resources.size();
    }

    template<typename ResourceType>
    inline bool ResourceCache<ResourceType>::Empty() const noexcept {
        return resources.empty();
    }

    template<typename ResourceType>
    inline void ResourceCache<ResourceType>::Clear() noexcept {
        resources.clear();
    }

    template<typename ResourceType>
    template<typename Loader, typename...Args>
    inline bool ResourceCache<ResourceType>::Load(resource_type id, Args&&...args) {
        static_assert(std::is_base_of_v<ResourceLoader<Loader, ResourceType>, Loader>, "Passed loader object isn't derived from ResourceLoader");
        if (resources.find(id) == resources.cend()) {
            std::shared_ptr<Resource> resource = Loader{}.Load(std::forward<Args>(args)...);
            if (resource.get() != nullptr) {
                resources[id] = std::move(resource);
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return true;
        }
    }

    template<typename ResourceType>
    template<typename Loader, typename...Args>
    inline bool ResourceCache<ResourceType>::Reload(resource_type id, Args&&...args) {
        Discard(id);
        return Load<Loader>(id, std::forward<Args>(args)...);
    }

    template<typename ResourceType>
    template<typename Loader, typename...Args>
    inline ResourceHandle<ResourceType> ResourceCache<ResourceType>::TemporaryHandle(Args&&...args) const {
        return { Loader{}.Load(std::forward<Args>(args)...) };
    }

    template<typename ResourceType>
    inline bool ResourceCache<ResourceType>::Contains(resource_type id) const noexcept {
        return (resources.find(id) != resources.cend());
    }

    template<typename ResourceType>
    inline void ResourceCache<ResourceType>::Discard(resource_type id) const noexcept {
        auto iter = resources.find(id);
        if (iter != resources.cend()) {
            resources.erase(iter);
        }
    }

    template<typename ResourceType>
    inline ResourceHandle<ResourceType> ResourceCache<ResourceType>::CreateHandle(resource_type id) const {
        auto iter = resources.find(id);
        if (iter != resources.cend()) {
            return ResourceHandle<ResourceType>{ iter->second };
        }
        else {
            return ResourceHandle<ResourceType>(nullptr);
        }
    }

}

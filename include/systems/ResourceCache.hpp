#pragma once
#ifndef VPSK_RESOURCE_CACHE_HPP
#define VPSK_RESOURCE_CACHE_HPP
#include "resources/ResourceHandle.hpp"
#include <unordered_map>
#include <string>
#include "util/HashedString.hpp"
#include "chrysocyon/signal/Delegate.hpp"

namespace vpsk {

    template<typename ResourceType>
    class ResourceCache;

    template<typename Loader, typename ResourceType>
    class ResourceLoader {
        friend class ResourceCache<ResourceType>;

        template<typename...Args>
        std::shared_ptr<ResourceType> Load(Args&&...args) {
            return static_cast<const Loader*>(this)->Load(std::forward<Args>(args)...);
        }

    };

    template<typename ResourceType>
    class ResourceCache {
        using container_type = std::unordered_map<HashedString::hash_type, std::shared_ptr<ResourceType>>;
    public:
        using size_type = typename container_type::size_type;
        using resource_type = HashedString;

        ResourceCache() noexcept = default;
        ResourceCache(const ResourceCache&) = delete;
        ResourceCache(ResourceCache&& other) noexcept = default;
        ResourceCache& operator=(const ResourceCache&) = delete;
        ResourceCache& operator=(ResourceCache&& other) noexcept = default;

        size_type Size() const noexcept;
        bool Empty() const noexcept;
        void Clear() noexcept;

        template<typename Loader, typename...Args>
        bool Load(resource_type id, Args&&...args);
        template<typename Loader, typename...Args>
        bool Reload(resource_type id, Args&&...args);
        template<typename Loader, typename...Args>
        ResourceHandle<ResourceType> TemporaryHandle(Args&&...args) const;

        ResourceHandle<ResourceType> CreateHandle(resource_type id) const;
        bool Contains(resource_type id) const noexcept;
        void Discard(resource_type id) const noexcept;

    private:
        container_type resources;
    };

}

#endif //!VPSK_RESOURCE_CACHE_HPP

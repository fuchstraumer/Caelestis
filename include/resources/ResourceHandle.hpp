#pragma once 
#ifndef VPSK_RESOURCE_HANDLE_HPP
#define VPSK_RESOURCE_HANDLE_HPP
#include <memory>
#include <utility>

namespace vpsk {

    template<typename ResourceType>
    class ResourceCache;

    template<typename ResourceType>
    class ResourceHandle final {
        friend class ResourceCache<ResourceType>;
        std::shared_ptr<ResourceType> resource;
        ResourceHandle(std::shared_ptr<ResourceType> _resource) noexcept : resource(std::move(_resource)) {}
    public:
        
        ResourceHandle(const ResourceHandle& other) noexcept : resource(other.resource) {}
        ResourceHandle(ResourceHandle&& other) noexcept : resource(std::move(other.resource)) {}

        ResourceHandle& operator=(const ResourceHandle& other) noexcept {
            resource = other.resource;
            return *this;
        }

        ResourceHandle& operator=(ResourceHandle&& other) noexcept {
            resource = std::move(other.resource);
            return *this;
        }

        const ResourceType& get() const noexcept {
            return *resource;
        }

        inline explicit operator const ResourceType&() const noexcept {
            return get();
        }

        inline const Resource* operator->() const noexcept {
            return resource.get();
        }

        explicit operator bool() const noexcept {
            return (resource.get() != nullptr);
        }
        
    };

}

#endif //!VPSK_RESOURCE_HANDLE_HPP

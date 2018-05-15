#pragma once
#ifndef VPSK_RESOURCE_LOADER_HPP
#define VPSK_RESOURCE_LOADER_HPP
#include <array>
#include <memory>
#include <thread>
#include <condition_variable>
#include <future>
#include <queue>
#include <mutex>
#include <string>
#include <unordered_map>
#include "ecs/signal/Delegate.hpp"

namespace vpsk {

    struct LoadRequest {
        std::string AbsolutePath;
        delegate_t<void(ResourceDataHandle&)> Signal;
    };

    // Specialized objects should inherit from this
    struct ResourceData {
        virtual ~ResourceData() = default;
        virtual void Load(const LoadRequest& request) = 0; 
    };
    
    using ResourceDataHandle = std::weak_ptr<ResourceData>;
    
    class ResourceLoader {
        ResourceLoader(const ResourceLoader&) = delete;
        ResourceLoader& operator=(const ResourceLoader&) = delete;
    public:

        ResourceLoader() noexcept {};


        void Add(const LoadRequest& item);
        ResourceDataHandle& GetResource(const std::string& absolute_file_path);

        static ResourceLoader& GetResourceLoader() noexcept;

    private:
    
        std::unordered_map<std::string, std::shared_ptr<ResourceData>> data;
        std::queue<LoadRequest> requests;
        std::recursive_mutex queueMutex;
        std::condition_variable cVar;
        std::atomic<bool> empty;
        std::array<std::thread, 2> workers;
    };

} // vpsk

#endif //!VPSK_RESOURCE_LOADER_HPP

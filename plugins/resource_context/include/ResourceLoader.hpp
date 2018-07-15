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
#include <functional>
#include "chrysocyon/signal/Delegate.hpp"

    using FactoryFunctor = delegate_t<void*(const char*)>;
    using SignalFunctor = delegate_t<void(void*)>;

    class ResourceLoader {
        ResourceLoader(const ResourceLoader&) = delete;
        ResourceLoader& operator=(const ResourceLoader&) = delete;
        ResourceLoader();
        ~ResourceLoader();
    public:

        void Subscribe(const std::string& file_type, FactoryFunctor func);
        void Load(const std::string& file_type, const std::string& file_path, SignalFunctor signal);
        void Unload(const std::string& file_type, const std::string& path);

        void Start();
        void Stop();

        static ResourceLoader& GetResourceLoader();

        struct ResourceData {
            void* Data;
            std::string FileType;
            std::string AbsoluteFilePath;
            size_t RefCount{ 0 };
        };

    private:

        struct loadRequest {
            loadRequest(ResourceData& dest) : destinationData(dest) {}
            ResourceData& destinationData;
            SignalFunctor signal;
            FactoryFunctor factory;
        };

        void workerFunction();

        std::unordered_map<std::string, FactoryFunctor> factories;
        std::unordered_map<std::string, ResourceData> resources;
        std::queue<loadRequest> requests;
        std::mutex queueMutex;
        std::condition_variable cVar;
        std::atomic<bool> shutdown{ false };
        std::array<std::thread, 2> workers;
    };


#endif //!VPSK_RESOURCE_LOADER_HPP

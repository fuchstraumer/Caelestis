#pragma once
#ifndef VPSK_RESOURCE_LOADER_HPP
#define VPSK_RESOURCE_LOADER_HPP
#include <array>
#include <memory>
#include <thread>
#include <condition_variable>
#include <future>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <functional>

    using FactoryFunctor = void*(*)(const char* fname);
    using DeleteFunctor = void(*)(void* obj_instance);
    using SignalFunctor = void(*)(void* state, void* data);

    class ResourceLoader {
        ResourceLoader(const ResourceLoader&) = delete;
        ResourceLoader& operator=(const ResourceLoader&) = delete;
        ResourceLoader();
        ~ResourceLoader();
    public:

        void Subscribe(const char* file_type, FactoryFunctor func, DeleteFunctor del_fn);
        void Load(const char* file_type, const char* file_path, void* requester, SignalFunctor signal);
        void Unload(const char* file_type, const char* path);

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

        // Used by ResourceContext: if a user uses the address of data we have already stored in here in the initial
        // data structure, re-use this systems address instead of copying it into our per-frame memory arena
        void* findAddress(const void* data_ptr);
        // unload or update the refcount of the given address
        void unloadAddress(const void* data_ptr);

        friend class ResourceContext;

        struct loadRequest {
            loadRequest(ResourceData& dest) : destinationData(dest), requester(nullptr) {}
            ResourceData& destinationData;
            void* requester; // state pointer of requesting object, if given
            SignalFunctor signal;
            FactoryFunctor factory;
        };

        void workerFunction();

        std::unordered_map<std::string, FactoryFunctor> factories;
        std::unordered_map<std::string, DeleteFunctor> deleters;
        std::unordered_map<std::string, ResourceData> resources;
        std::list<loadRequest> requests;
        std::recursive_mutex queueMutex;
        std::condition_variable_any cVar;
        std::atomic<bool> shutdown{ false };
        std::array<std::thread, 2> workers;
    };


#endif //!VPSK_RESOURCE_LOADER_HPP

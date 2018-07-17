#include "ResourceLoader.hpp"
#include <experimental/filesystem>
#ifdef _MSC_VER
#include <execution>
#endif // _MSC_VER

    ResourceLoader::ResourceLoader() {
        Start();
    }

    ResourceLoader::~ResourceLoader() {
        Stop();
    }

    void ResourceLoader::Subscribe(const std::string& file_type, FactoryFunctor func, DeleteFunctor del_fn) {
        factories[file_type] = func;
        deleters[file_type] = del_fn;
    }

    void ResourceLoader::Load(const std::string& file_type, const std::string& file_path, void* _requester, SignalFunctor signal) {
        namespace fs = std::experimental::filesystem;
        fs::path fs_file_path = fs::absolute(fs::path(file_path));
        const std::string absolute_path = fs_file_path.string();

        if (resources.count(absolute_path) != 0) {
            // Call the signal saying this resource is already loaded
            ++resources.at(absolute_path).RefCount;
            signal(_requester, resources.at(absolute_path).Data);
            return;
        }
        
        if (!fs::exists(fs_file_path)) {
            throw std::runtime_error("Given path to a resource does not exist.");
        }

        if (factories.count(file_type) == 0) {
            throw std::domain_error("Tried to load resource type for which there is no factory!");
        }

        if (deleters.count(file_type) == 0) {
            throw std::domain_error("No deleter function for current file type!");
        }

        ResourceData data;      
        data.FileType = file_type;
        data.AbsoluteFilePath = absolute_path;
        data.RefCount = 1;
        auto iter = resources.emplace(absolute_path, data);

        loadRequest req(iter.first->second);
        req.requester = _requester;
        req.signal = signal;
        req.factory = factories.at(file_type);
        {
            std::lock_guard<std::mutex> guard(queueMutex);
            requests.push(req);
        }
        cVar.notify_all();

    }   

    void ResourceLoader::Unload(const std::string& file_type, const std::string& path) {
        if (resources.count(path) != 0) {
            std::lock_guard<std::mutex> guard(queueMutex);
            auto iter = resources.find(path);
            --iter->second.RefCount;
            if (iter->second.RefCount == 0) {
                deleters.at(iter->first)(iter->second.Data);
            }
            resources.erase(iter);
        }
    }

    ResourceLoader & ResourceLoader::GetResourceLoader() {
        static ResourceLoader loader;
        return loader;
    }

    void ResourceLoader::Start() {
        workers[0] = std::thread(&ResourceLoader::workerFunction, this);
        workers[1] = std::thread(&ResourceLoader::workerFunction, this);

        shutdown = false;
    }

    void ResourceLoader::Stop() {
        shutdown = true;      
        
        if (workers[0].joinable()) {
            workers[0].join();
        }
        
        if (workers[1].joinable()) {
            workers[1].join();
        }

        for (auto& rsrc : resources) {
            deleters.at(rsrc.first)(rsrc.second.Data);
        }

    }

    void* ResourceLoader::findAddress(const void * data_ptr) {
        void* result = nullptr;
        auto find_fn = [data_ptr, &result](decltype(resources)::value_type& data) {
            if (data.second.Data == data_ptr) {
                result = data.second.Data;
                data.second.RefCount++;
            }
        };
#ifdef _MSC_VER
        std::for_each(std::execution::par_unseq, std::begin(resources), std::end(resources), find_fn);
#else
        std::for_each(std::begin(resources), std::end(resources), find_fn);
#endif        
        return result;
    }

    void ResourceLoader::unloadAddress(const void * data_ptr) {
        auto find_fn = [data_ptr](const decltype(resources)::value_type& data) {
            return (data_ptr == data.second.Data);
        };
#ifdef _MSC_VER
        auto iter = std::find_if(std::execution::par_unseq, std::begin(resources), std::end(resources), find_fn);
#else
        auto iter = std::find_if(std::begin(resources), std::end(resources), find_fn);
#endif   
        if (iter != std::end(resources)) {
            Unload(iter->second.FileType, iter->second.AbsoluteFilePath);
        }
    }

    void ResourceLoader::workerFunction() {
        while (!shutdown) {
            std::unique_lock<std::mutex> lock{queueMutex};
            cVar.wait(lock, [&]{ 
                return !requests.empty() || shutdown;
            });
            loadRequest request = requests.front();
            requests.pop();
            lock.unlock();
            // proceed to load.
            request.destinationData.Data = request.factory(request.destinationData.AbsoluteFilePath.c_str());
            request.signal(request.requester, request.destinationData.Data);
        }
    }


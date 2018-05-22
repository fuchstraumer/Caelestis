#include "renderer/systems/ResourceLoader.hpp"
#include <experimental/filesystem>
namespace vpsk {

    ResourceLoader::ResourceLoader() {
        Start();
    }

    void ResourceLoader::Subscribe(const std::string& file_type, FactoryFunctor func) {
        factories[file_type] = func;
    }

    void ResourceLoader::Load(const std::string& file_type, const std::string& file_path, SignalFunctor signal) {
        namespace fs = std::experimental::filesystem;
        fs::path fs_file_path = fs::absolute(fs::path(file_path));
        const std::string absolute_path = fs_file_path.string();

        if (resources.count(absolute_path) != 0) {
            // Call the signal saying this resource is already loaded
            ++resources.at(absolute_path).RefCount;
            signal(resources.at(absolute_path).Data);
        }
        
        if (!fs::exists(fs_file_path)) {
            throw std::runtime_error("Given path to a resource does not exist.");
        }

        if (factories.count(file_type) == 0) {
            throw std::domain_error("Tried to load resource type for which there is no factory!");
        }

        ResourceData data;
        data.FileType = file_type;
        data.AbsoluteFilePath = absolute_path;
        data.RefCount = 1;
        auto iter = resources.emplace(absolute_path, data);

        loadRequest req(iter.first->second);
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
                delete iter->second.Data;
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
            request.signal(request.destinationData.Data);
        }
    }

}

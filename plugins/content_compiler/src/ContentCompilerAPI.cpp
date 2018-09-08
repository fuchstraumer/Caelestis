#include "ContentCompilerAPI.hpp"
#include "MeshData.hpp"
#include "AssimpMeshImporter.hpp"
#include "CoreAPIs.hpp"
#include "PluginAPI.hpp"
#include "resource_context/include/ResourceContextAPI.hpp"
#include "application_context/include/AppContextAPI.hpp"
#include <vector>
#include <string>
#include "easylogging++.h"
INITIALIZE_NULL_EASYLOGGINGPP

static ResourceContext_API* resource_api = nullptr;
static ApplicationContext_API* application_api = nullptr;
static std::vector<std::string> loadedFiles;

static void* LoadMeshData(const char* fname) {
    return AssimpLoadMeshData(fname, true);
}

static void DestroyMeshData_void(void* ptr) {
    MeshData* data = reinterpret_cast<MeshData*>(ptr);
    MeshData::DestroyMeshData(data);
}

static uint32_t GetID() {
    return RESOURCE_CONTEXT_API_ID;
}

static const char* GetName() {
    return "AssetPipeline";
}

static void WriteLoadedLog() {
    LOG(INFO) << "Asset Pipeline plugin loaded.";
}

static void Load(GetEngineAPI_Fn fn) {
    resource_api = reinterpret_cast<ResourceContext_API*>(fn(RESOURCE_CONTEXT_API_ID));
    application_api = reinterpret_cast<ApplicationContext_API*>(fn(APPLICATION_CONTEXT_API_ID));
    el::Helpers::setStorage(*reinterpret_cast<el::base::type::StoragePointer*>(application_api->GetLoggingStoragePointer()));
    WriteLoadedLog();
    resource_api->RegisterFileTypeFactory("ASSET_PIPELINE_MESH", LoadMeshData, DestroyMeshData_void);
}

static void Unload() {
    for (auto& f : loadedFiles) {
        resource_api->UnloadFile("ASSET_PIPELINE_MESH", f.c_str());
    }
}

static void LogicalUpdate() {

}

static void TimeDependentUpdate(double dt) {

}

MeshData* LoadMeshData(const char* fname, bool interleaved) {
    return AssimpLoadMeshData(fname, interleaved);
}

void LoadMeshDataAsync(const char* fname, bool interleaved, void* requester, ContentCompiler_API::mesh_loaded_signal_t signal) {
    loadedFiles.emplace_back(std::string(fname));
    resource_api->LoadFile("ASSET_PIPELINE_MESH", fname, requester, signal, nullptr);
}

static Plugin_API* GetCoreAPI() {
    static Plugin_API api{ nullptr };
    api.PluginID = GetID;
    api.PluginName = GetName;
    api.Load = Load;
    api.Unload = Unload;
    api.LogicalUpdate = LogicalUpdate;
    api.TimeDependentUpdate = TimeDependentUpdate;
    return &api;
}

static ContentCompiler_API* GetAssetAPI() {
    static ContentCompiler_API api{ nullptr };
    api.LoadMeshFromFileAssimp = LoadMeshData;
    api.AsyncLoadMeshFromFileAssimp = LoadMeshDataAsync;
    api.DestroyMeshData = MeshData::DestroyMeshData;
    return &api;
}

PLUGIN_API void* GetPluginAPI(uint32_t api_id) {
    switch (api_id) {
    case 0:
        return GetCoreAPI();
    case ASSET_PIPELINE_PLUGIN_API_ID:
        return GetAssetAPI();
    default:
        return nullptr;
    }
}

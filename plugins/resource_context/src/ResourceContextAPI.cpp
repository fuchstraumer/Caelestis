#include "ResourceContextAPI.hpp"
#include "PluginAPI.hpp"
#include "CoreAPIs.hpp"
#include "application_context/include/AppContextAPI.hpp"
#include "renderer_context/include/RendererContextAPI.hpp"
#include "renderer_context/include/core/RendererContext.hpp"
#include "ResourceContext.hpp"
#include "ResourceLoader.hpp"
#include "Allocator.hpp"
#include "PipelineCache.hpp"
#include "PhysicalDevice.hpp"
#include "easylogging++.h"
INITIALIZE_NULL_EASYLOGGINGPP

static RendererContext* rendererContext = nullptr;
static ApplicationContext_API* AppContextAPI = nullptr;
static RendererContext_API* RendererAPI = nullptr;
static ResourceContext* resourceContext = nullptr;

static void BeginResizeCallback(uint64_t handle, uint32_t w, uint32_t h) {
    auto& loader = ResourceLoader::GetResourceLoader();
    loader.Stop();
    resourceContext->Update();
    resourceContext->FlushStagingBuffers();
}

static void CompleteResizeCallback(uint64_t handle, uint32_t w, uint32_t h) {
    auto& loader = ResourceLoader::GetResourceLoader();
    loader.Start();
}

static uint32_t GetID() {
    return RESOURCE_CONTEXT_API_ID;
}

static const char* GetName() {
    return "ResourceContext";
}

static void SetEasyloggingRepositoryUsingAppContext() {
    el::Helpers::setStorage(*reinterpret_cast<el::base::type::StoragePointer*>(AppContextAPI->GetLoggingStoragePointer()));
    vpr::SetLoggingRepository_VprAlloc(AppContextAPI->GetLoggingStoragePointer());
}

static void WriteLoadedLog() {
    std::string device_name = rendererContext->PhysicalDevices[0]->GetProperties().deviceName;
    uint32_t version_int = rendererContext->PhysicalDevices[0]->GetProperties().driverVersion;
    std::string version_str = "v" + std::to_string(VK_VERSION_MAJOR(version_int)) + "." + std::to_string(VK_VERSION_MINOR(version_int)) + "." + std::to_string(VK_VERSION_PATCH(version_int));
    std::string creation_info_log = "ResourceContext plugin loaded successfully, created resource system on device " + device_name + " with driver " + version_str;
    LOG(INFO) << creation_info_log;
}

static void Load(GetEngineAPI_Fn fn) {
    AppContextAPI = reinterpret_cast<ApplicationContext_API*>(fn(APPLICATION_CONTEXT_API_ID));
    RendererAPI = reinterpret_cast<RendererContext_API*>(fn(RENDERER_CONTEXT_API_ID));
    SetEasyloggingRepositoryUsingAppContext();
    SwapchainCallbacks_API api{nullptr};
    api.BeginSwapchainResize = BeginResizeCallback;
    api.CompleteSwapchainResize = CompleteResizeCallback;
    RendererAPI->RegisterSwapchainCallbacks(&api);
    rendererContext = RendererAPI->GetContext();
    resourceContext = new ResourceContext(rendererContext->LogicalDevice, rendererContext->PhysicalDevices[0]);
    // Write info about active system and active hardware.
    WriteLoadedLog();
}

static void Unload() {
    if (resourceContext) {
        delete resourceContext;
    }
}

static void LogicalUpdate() {

}

static void Update(double dt) {

}

inline memory_type convertToMemoryType(const uint32_t input) {
    switch (input) {
    case 0:
        return memory_type::INVALID_MEMORY_TYPE;
    case 1:
        return memory_type::HOST_VISIBLE;
    case 2:
        return memory_type::HOST_VISIBLE_AND_COHERENT;
    case 3:
        return memory_type::DEVICE_LOCAL;
    case 4:
        return memory_type::SPARSE;
    default:
        return memory_type::INVALID_MEMORY_TYPE;
    }
}

VulkanResource* CreateBuffer(const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const size_t num_data, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data) {
    return resourceContext->CreateBuffer(info, view_info, num_data, initial_data, convertToMemoryType(_memory_type), user_data);
}

VulkanResource* CreateNamedBuffer(const char* name, const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const size_t num_data, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data) {
    return resourceContext->CreateNamedBuffer(name, info, view_info, num_data, initial_data, convertToMemoryType(_memory_type), user_data);
}

void SetBufferData(VulkanResource* dest_buffer, const size_t num_data, const gpu_resource_data_t* data) {
    resourceContext->SetBufferData(dest_buffer, num_data, data);
}

void FillBuffer(VulkanResource* dest, const uint32_t value, const size_t offset, const size_t fill_size) {
    resourceContext->FillBuffer(dest, value, offset, fill_size);
}

VulkanResource* CreateImage(const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const size_t num_data, const gpu_image_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data) {
    return resourceContext->CreateImage(info, view_info, num_data, initial_data, convertToMemoryType(_memory_type), user_data);
}

VulkanResource* CreateNamedImage(const char* name, const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const size_t num_data, const gpu_image_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data) {
    return resourceContext->CreateNamedImage(name, info, view_info, num_data, initial_data, convertToMemoryType(_memory_type), user_data);
}

void SetImageData(VulkanResource* image, const size_t num_data, const gpu_image_resource_data_t* data) {
    resourceContext->SetImageData(image, num_data, data);
}

VulkanResource* CreateSampler(const struct VkSamplerCreateInfo* info, void* user_data) {
    return resourceContext->CreateSampler(info, user_data);
}

void CopyResource(VulkanResource* src, VulkanResource* dst) {
    resourceContext->CopyResource(src, dst);
}

VulkanResource* CreateResourceCopy(VulkanResource* src) {
    return resourceContext->CreateResourceCopy(src);
}

void DestroyResource(VulkanResource* resource) {
    resourceContext->DestroyResource(resource);
}

void CompletePendingTransfers() {
    resourceContext->Update();
}

void FlushStagingBuffers() {
    resourceContext->FlushStagingBuffers();
}

void RegisterFileTypeFactory(const char* file_type, ResourceContext_API::factory_function_t load_fn, ResourceContext_API::deleter_function_t del_fn) {
    auto& loader = ResourceLoader::GetResourceLoader();
    loader.Subscribe(file_type, load_fn, del_fn);
}

void LoadFile(const char* file_type, const char* file_name, void* requester, ResourceContext_API::signal_function_t signal_fn, void* user_data) {
    auto& loader = ResourceLoader::GetResourceLoader();
    loader.Load(file_type, file_name, requester, signal_fn, user_data);
}

void UnloadFile(const char* file_type, const char* fname) {
    auto& loader = ResourceLoader::GetResourceLoader();
    loader.Unload(file_type, fname);
}

uint64_t GetVkDevice() {
    return (uint64_t)rendererContext->LogicalDevice;
}

static Plugin_API* GetCoreAPI() {
    static Plugin_API api{ nullptr };
    api.PluginID = GetID;
    api.PluginName = GetName;
    api.Load = Load;
    api.Unload = Unload;
    api.LogicalUpdate = LogicalUpdate;
    api.TimeDependentUpdate = Update;
    return &api;
}

static ResourceContext_API* GetResourceAPI() {
    static ResourceContext_API api{ nullptr };
    api.CreateBuffer = CreateBuffer;
    api.CreateNamedBuffer = CreateNamedBuffer;
    api.SetBufferData = SetBufferData;
    api.CreateImage = CreateImage;
    api.CreateNamedImage = CreateNamedImage;
    api.CreateSampler = CreateSampler;
    api.CopyResource = CopyResource;
    api.CreateResourceCopy = CreateResourceCopy;
    api.DestroyResource = DestroyResource;
    api.CompletePendingTransfers = CompletePendingTransfers;
    api.FlushStagingBuffers = FlushStagingBuffers;
    api.RegisterFileTypeFactory = RegisterFileTypeFactory;
    api.LoadFile = LoadFile;
    api.UnloadFile = UnloadFile;
    api.GetVkDevice = GetVkDevice;
    return &api;
}

PLUGIN_API void* GetPluginAPI(uint32_t api_id) {
    switch (api_id) {
    case 0:
        return GetCoreAPI();
    case RESOURCE_CONTEXT_API_ID:
        return GetResourceAPI();
    default:
        return nullptr;
    }
}

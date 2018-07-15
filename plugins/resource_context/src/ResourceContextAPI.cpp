#include "ResourceContextAPI.hpp"
#include "PluginAPI.hpp"
#include "CoreAPIs.hpp"
#include "application_context/include/AppContextAPI.hpp"
#include "renderer_context/include/RendererContextAPI.hpp"
#include "renderer_context/include/core/RendererContext.hpp"
#include "ResourceContext.hpp"
#include "ResourceLoader.hpp"

static RendererContext* rendererContext = nullptr;
static ApplicationContext_API* AppContextAPI = nullptr;
static RendererContext_API* RendererAPI = nullptr;
static ResourceContext* resourceContext = nullptr;

static void BeginResizeCallback(uint32_t handle, uint32_t w, uint32_t h) {
    auto& loader = ResourceLoader::GetResourceLoader();
    loader.Stop();
    resourceContext->Update();
    resourceContext->FlushStagingBuffers();
    delete resourceContext;
}

static void CompleteResizeCallback(uint32_t handle, uint32_t w, uint32_t h) {
    resourceContext = new ResourceContext(rendererContext->LogicalDevice, rendererContext->PhysicalDevices[0]);
    auto& loader = ResourceLoader::GetResourceLoader();
    loader.Start();
}

static uint32_t GetID() {
    return RESOURCE_CONTEXT_API_ID;
}

static const char* GetName() {
    return "ResourceContext";
}

static void Load(GetEngineAPI_Fn fn) {
    AppContextAPI = reinterpret_cast<ApplicationContext_API*>(fn(APPLICATION_CONTEXT_API_ID));
    RendererAPI = reinterpret_cast<RendererContext_API*>(fn(RENDERER_CONTEXT_API_ID));
    SwapchainCallbacks_API api{nullptr};
    api.BeginSwapchainResize = BeginResizeCallback;
    api.CompleteSwapchainResize = CompleteResizeCallback;
    RendererAPI->RegisterSwapchainCallbacks(&api);
    rendererContext = RendererAPI->GetContext();
    resourceContext = new ResourceContext(rendererContext->LogicalDevice, rendererContext->PhysicalDevices[0]);
}

static void Unload() {
    if (resourceContext) {
        delete resourceContext;
    }
}

static void LogicalUpdate() {

}

static void Update(float dt) {

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

VulkanResource* CreateBuffer(const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data) {
    return resourceContext->CreateBuffer(info, view_info, initial_data, convertToMemoryType(_memory_type), user_data);
}

VulkanResource* CreateNamedBuffer(const char* name, const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data) {
    return resourceContext->CreateNamedBuffer(name, info, view_info, initial_data, convertToMemoryType(_memory_type), user_data);
}

void SetBufferData(VulkanResource* dest_buffer, const gpu_resource_data_t* data) {
    resourceContext->SetBufferData(dest_buffer, data);
}

VulkanResource* CreateImage(const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data) {
    return resourceContext->CreateImage(info, view_info, initial_data, convertToMemoryType(_memory_type), user_data);
}

VulkanResource* CreateNamedImage(const char* name, const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data) {
    return resourceContext->CreateNamedImage(name, info, view_info, initial_data, convertToMemoryType(_memory_type), user_data);
}

void SetImageData(VulkanResource* image, const gpu_resource_data_t* data) {
    resourceContext->SetImageData(image, data);
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

void RegisterFileTypeFactory(const char* file_type, ResourceContext_API::factory_function_t load_fn) {
    auto& loader = ResourceLoader::GetResourceLoader();
    auto fn = [load_fn](const char* fname) {
        return load_fn(fname);
    };
    loader.Subscribe(file_type, FactoryFunctor::create<>(fn));
}

void LoadFile(const char* file_type, const char* file_name, ResourceContext_API::signal_function_t signal_fn) {
    auto& loader = ResourceLoader::GetResourceLoader();
    auto fn = [&](void* ptr) {
        signal_fn(ptr);
    };

    loader.Load(file_type, file_name, SignalFunctor::create<>(fn));
}

void UnloadFile(const char* file_type, const char* fname) {
    auto& loader = ResourceLoader::GetResourceLoader();
    loader.Unload(file_type, fname);
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

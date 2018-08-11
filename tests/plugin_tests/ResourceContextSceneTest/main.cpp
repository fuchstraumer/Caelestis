 #include "VulkanComplexScene.hpp"
#include "../../../plugins/renderer_context/include/RendererContextAPI.hpp"
#include "../../../plugins//renderer_context/include/core/RendererContext.hpp"
#include "../../../plugins/resource_context/include/ResourceContextAPI.hpp"
#include "../../../plugin_manager/include/PluginManager.hpp"
#include "../../..//plugin_manager/include/CoreAPIs.hpp"
#include "../../../plugins/application_context/include/AppContextAPI.hpp"
#include "PipelineCache.hpp"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

static RendererContext* context = nullptr;
static ResourceContext_API* resource_api = nullptr;

static void objFileLoadedCallback(void* scene_ptr, void* data_ptr) {
    reinterpret_cast<VulkanComplexScene*>(scene_ptr)->CreateHouseMesh(data_ptr);
}

static void jpegLoadedCallback(void* scene_ptr, void* data_ptr) {
    reinterpret_cast<VulkanComplexScene*>(scene_ptr)->CreateHouseTexture(data_ptr);
}

static void skyboxLoadedCallback(void* scene_ptr, void* data) {
    reinterpret_cast<VulkanComplexScene*>(scene_ptr)->CreateSkyboxTexture(data);
}

static void BeginResizeCallback(uint64_t handle, uint32_t width, uint32_t height) {
    auto& scene = VulkanComplexScene::GetScene();
    scene.Destroy();
}

static void CompleteResizeCallback(uint64_t handle, uint32_t width, uint32_t height) {
    auto& scene = VulkanComplexScene::GetScene();
    scene.Construct(RequiredVprObjects{ context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain }, resource_api);
    resource_api->LoadFile("OBJ", "House.obj", &scene, objFileLoadedCallback);
    resource_api->LoadFile("JPEG", "House.png", &scene, jpegLoadedCallback);
    resource_api->LoadFile("DDS", "Starbox.dds", &scene, skyboxLoadedCallback);
}

int main(int argc, char* argv[]) {
    PluginManager& manager = PluginManager::GetPluginManager();
    manager.LoadPlugin("application_context.dll");
    ApplicationContext_API* appl_api = reinterpret_cast<ApplicationContext_API*>(manager.RetrieveAPI(APPLICATION_CONTEXT_API_ID));
    void* storage_ptr = appl_api->GetLoggingStoragePointer();
    el::Helpers::setStorage(*reinterpret_cast<el::base::type::StoragePointer*>(storage_ptr));
    vpr::SetLoggingRepository_VprResource(storage_ptr);
    manager.LoadPlugin("renderer_context.dll");
    manager.LoadPlugin("resource_context.dll");

    RendererContext_API* renderer_api = reinterpret_cast<RendererContext_API*>(manager.RetrieveAPI(RENDERER_CONTEXT_API_ID));
    context = renderer_api->GetContext();
    resource_api = reinterpret_cast<ResourceContext_API*>(manager.RetrieveAPI(RESOURCE_CONTEXT_API_ID));
    resource_api->RegisterFileTypeFactory("OBJ", &VulkanComplexScene::LoadObjFile, &VulkanComplexScene::DestroyObjFileData);
    resource_api->RegisterFileTypeFactory("JPEG", &VulkanComplexScene::LoadJpegImage, &VulkanComplexScene::DestroyJpegFileData);
    resource_api->RegisterFileTypeFactory("DDS", &VulkanComplexScene::LoadCompressedTexture, &VulkanComplexScene::DestroyCompressedTextureData);

    auto& scene = VulkanComplexScene::GetScene();
    scene.Construct(RequiredVprObjects{ context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain }, resource_api);
    resource_api->LoadFile("OBJ", "House.obj", &scene, objFileLoadedCallback);
    resource_api->LoadFile("JPEG", "House.png", &scene, jpegLoadedCallback);
    resource_api->LoadFile("DDS", "Starbox.dds", &scene, skyboxLoadedCallback);

    SwapchainCallbacks_API callbacks{ nullptr };
    callbacks.BeginSwapchainResize = BeginResizeCallback;
    callbacks.CompleteSwapchainResize = CompleteResizeCallback;
    renderer_api->RegisterSwapchainCallbacks(&callbacks);

    Plugin_API* renderer_context_core_api = reinterpret_cast<Plugin_API*>(manager.RetrieveBaseAPI(RENDERER_CONTEXT_API_ID));
    //scene.WaitForAllLoaded();

    while (!renderer_api->WindowShouldClose()) {
        renderer_context_core_api->LogicalUpdate();
        resource_api->CompletePendingTransfers();
        scene.Render(nullptr);
    }

    resource_api->UnloadFile("OBJ", "House.obj");
    resource_api->UnloadFile("JPEG", "House.jpg");
    resource_api->UnloadFile("DDS", "StarboxMips.dds");

}

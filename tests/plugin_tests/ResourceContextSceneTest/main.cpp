#include "VulkanComplexScene.hpp"
#include "../../../plugins/renderer_context/include/RendererContextAPI.hpp"
#include "../../../plugins//renderer_context/include/core/RendererContext.hpp"
#include "../../../plugins/resource_context/include/ResourceContextAPI.hpp"
#include "../../../plugin_manager/include/PluginManager.hpp"
#include "../../..//plugin_manager/include/CoreAPIs.hpp"

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

int main(int argc, char* argv[]) {
    PluginManager& manager = PluginManager::GetPluginManager();
    manager.LoadPlugin("application_context.dll");
    manager.LoadPlugin("renderer_context.dll");
    manager.LoadPlugin("resource_context.dll");

    RendererContext_API* renderer_api = reinterpret_cast<RendererContext_API*>(manager.RetrieveAPI(RENDERER_CONTEXT_API_ID));
    context = renderer_api->GetContext();
    resource_api = reinterpret_cast<ResourceContext_API*>(manager.RetrieveAPI(RESOURCE_CONTEXT_API_ID));
    resource_api->RegisterFileTypeFactory("OBJ", &VulkanComplexScene::LoadObjFile);
    resource_api->RegisterFileTypeFactory("JPEG", &VulkanComplexScene::LoadJpegImage);
    resource_api->RegisterFileTypeFactory("DDS", &VulkanComplexScene::LoadCompressedTexture);

    auto& scene = VulkanComplexScene::GetScene();
    scene.Construct(RequiredVprObjects{ context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain }, resource_api);
    resource_api->LoadFile("OBJ", "House.obj", &scene, objFileLoadedCallback);
    resource_api->LoadFile("JPEG", "House.jpg", &scene, jpegLoadedCallback);
    resource_api->LoadFile("DDS", "StarboxMips.dds", &scene, skyboxLoadedCallback);

    scene.WaitForAllLoaded();
}

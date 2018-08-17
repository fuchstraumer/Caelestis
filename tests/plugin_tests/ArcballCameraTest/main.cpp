#include "ArcballScene.hpp"
#include "../../../plugin_manager/include/PluginManager.hpp"
#include "../../../plugins/renderer_context/include/RendererContextAPI.hpp"
#include "../../../plugins//renderer_context/include/core/RendererContext.hpp"
#include "../../../plugins/resource_context/include/ResourceContextAPI.hpp"
#include "../../..//plugin_manager/include/CoreAPIs.hpp"
#include "../../../plugins/application_context/include/AppContextAPI.hpp"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP
#include "vpr/PipelineCache.hpp"
#include "vpr/Swapchain.hpp"
#include "ArcballHelper.hpp"
#include "Camera.hpp"
#include "ArcballCameraController.hpp"
#include "imgui.h"
#include "ImGuiWrapper.hpp"
#include "glm/gtc/matrix_transform.hpp"
#ifndef __APPLE_CC__
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif

static RendererContext* context = nullptr;
static ResourceContext_API* resource_api = nullptr;
static RequiredInitialData initial_scene_data{ nullptr };
static VkRect2D render_area;

static const std::string HouseObjFile(fs::absolute("House.obj").string());
static const std::string HousePngFile(fs::absolute("House.png").string()); 

static void objFileLoadedCallback(void* scene_ptr, void* data_ptr) {
    reinterpret_cast<ArcballScene*>(scene_ptr)->CreateHouseMesh(data_ptr);
}

static void pngLoadedCallback(void* scene_ptr, void* data_ptr) {
    reinterpret_cast<ArcballScene*>(scene_ptr)->CreateHouseTexture(data_ptr);
}

static void BeginResizeCallback(uint64_t handle, uint32_t width, uint32_t height) {
    auto& scene = ArcballScene::GetScene();
    scene.Destroy();
}

static void CompleteResizeCallback(uint64_t handle, uint32_t width, uint32_t height) {
    auto& scene = ArcballScene::GetScene();
    scene.Construct(RequiredVprObjects{ context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain }, &initial_scene_data);
    resource_api->LoadFile("OBJ", HouseObjFile.c_str(), &scene, objFileLoadedCallback, nullptr);
    resource_api->LoadFile("PNG", HousePngFile.c_str(), &scene, pngLoadedCallback, nullptr);
    render_area = VkRect2D{
        VkOffset2D{ 0, 0 },
        VkExtent2D{ context->Swapchain->Extent() }
    };
}

int main(int argc, char* argv[]) {
    PluginManager& manager = PluginManager::GetPluginManager();

#ifndef __APPLE_CC__
    manager.LoadPlugin("application_context.dll");
#else
    manager.LoadPlugin("libapplication_context.dylib");
#endif

    ApplicationContext_API* appl_api = reinterpret_cast<ApplicationContext_API*>(manager.RetrieveAPI(APPLICATION_CONTEXT_API_ID));
    void* storage_ptr = appl_api->GetLoggingStoragePointer();
    el::Helpers::setStorage(*reinterpret_cast<el::base::type::StoragePointer*>(storage_ptr));
    vpr::SetLoggingRepository_VprResource(storage_ptr);

#ifndef __APPLE_CC__
    manager.LoadPlugin("renderer_context.dll");
    manager.LoadPlugin("resource_context.dll");
#else
    manager.LoadPlugin("librenderer_context.dylib");
    manager.LoadPlugin("libresource_context.dylib");
#endif

    RendererContext_API* renderer_api = reinterpret_cast<RendererContext_API*>(manager.RetrieveAPI(RENDERER_CONTEXT_API_ID));
    initial_scene_data.rendererAPI = renderer_api;
    Plugin_API* renderer_core_api = reinterpret_cast<Plugin_API*>(manager.RetrieveBaseAPI(RENDERER_CONTEXT_API_ID));
    context = renderer_api->GetContext();
    RequiredVprObjects vpr_objects{ context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain };

    resource_api = reinterpret_cast<ResourceContext_API*>(manager.RetrieveAPI(RESOURCE_CONTEXT_API_ID));
    resource_api->RegisterFileTypeFactory("OBJ", &ArcballScene::LoadObj, &ArcballScene::DestroyObj);
    resource_api->RegisterFileTypeFactory("PNG", &ArcballScene::LoadPNG, &ArcballScene::DestroyPNG);

    auto& gui = ImGuiWrapper::GetImGuiWrapper();

    auto& scene = ArcballScene::GetScene();
    initial_scene_data.resourceAPI = resource_api;
    initial_scene_data.gui = &gui;
    PerspectiveCamera cam(60.0f, static_cast<float>(context->Swapchain->Extent().width), 
        static_cast<float>(context->Swapchain->Extent().height), 0.1f, 1000.0f);
    initial_scene_data.camera = &cam;
    Spatial object;
    object.LocalTransform = glm::mat4(1.0f);
    object.WorldTransform = glm::mat4(1.0f);
    object.Position = glm::vec3(0.0f);
    object.Scale = glm::vec3(1.0f);
    object.Orientation = cam.Orientation();

    ArcballCameraController controller(&object);

    scene.Construct(vpr_objects, &initial_scene_data);

    SwapchainCallbacks_API callbacks{ nullptr };
    callbacks.BeginSwapchainResize = BeginResizeCallback;
    callbacks.CompleteSwapchainResize = CompleteResizeCallback;
    renderer_api->RegisterSwapchainCallbacks(&callbacks);

    resource_api->LoadFile("OBJ", HouseObjFile.c_str(), &scene, objFileLoadedCallback, nullptr);
    resource_api->LoadFile("PNG", HousePngFile.c_str(), &scene, pngLoadedCallback, nullptr);

    render_area = VkRect2D{
        VkOffset2D{ 0, 0 },
        VkExtent2D{ context->Swapchain->Extent() }
    };

    auto update_model_matrix = [&](const glm::quat& q) {
        const glm::mat4 T = glm::translate(glm::mat4(1.0f), object.Position);
        const glm::mat4 R = glm::toMat4(object.Orientation);
        const glm::mat4 S = glm::scale(glm::mat4(1.0f), object.Scale);
        return T * R * S;
    };

    while (!renderer_api->WindowShouldClose()) {
        renderer_core_api->LogicalUpdate();
        gui.NewFrame();
        ImGui::ShowMetricsWindow();
        controller.Update(&cam, render_area);
        auto& helper = ArcballHelper::GetArcballHelper();
        helper.Update(controller);
        scene.UpdateHouseModelMatrix(update_model_matrix(object.Orientation));
        gui.EndImGuiFrame();
        resource_api->CompletePendingTransfers();
        scene.Render(nullptr);
    }
}
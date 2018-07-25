#include <chrono>
#include <thread>
#include "../../../plugins/renderer_context/include/RendererContextAPI.hpp"
#include "../../../plugins//renderer_context/include/core/RendererContext.hpp"
#include "../../../plugins/resource_context/include/ResourceContextAPI.hpp"
#include "../../../plugin_manager/include/PluginManager.hpp"
#include "../../..//plugin_manager/include/CoreAPIs.hpp"
#include "../../../plugins/application_context/include/AppContextAPI.hpp"
#include "../../../plugins/tethersim_plugin/include/TetherSimAPI.hpp"
#include "vpr/PipelineCache.hpp"
#include "vpr/CommandPool.hpp"
#include "TetherSimScene.hpp"
#include "easylogging++.h"
#include "imgui/imgui.h"
#include "ImGuiWrapper.hpp"
INITIALIZE_EASYLOGGINGPP

RendererContext* context = nullptr;
StartingData data{ nullptr, nullptr, "" };

static void BeginResizeCallback(uint32_t handle, uint32_t width, uint32_t height) {
    auto& gui = ImGuiWrapper::GetImGuiWrapper();
    gui.Destroy();
    auto& scene = TetherSimScene::GetScene();
    scene.Destroy();
}

static void CompleteResizeCallback(uint32_t handle, uint32_t width, uint32_t height) {
    auto& scene = TetherSimScene::GetScene();
    scene.Construct(RequiredVprObjects{ context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain }, &data);
}

int main(int argc, char* argv[]) {
    PluginManager& manager = PluginManager::GetPluginManager();
    manager.LoadPlugin("application_context.dll");
    ApplicationContext_API* appl_api = reinterpret_cast<ApplicationContext_API*>(manager.RetrieveAPI(APPLICATION_CONTEXT_API_ID));
    void* storage_ptr = appl_api->GetLoggingStoragePointer();
    el::Helpers::setStorage(*reinterpret_cast<el::base::type::StoragePointer*>(storage_ptr));
    vpr::SetLoggingRepository_VprResource(storage_ptr);
    vpr::SetLoggingRepository_VprCommand(storage_ptr);
    manager.LoadPlugin("renderer_context.dll");
    manager.LoadPlugin("resource_context.dll");

    RendererContext_API* renderer_api = reinterpret_cast<RendererContext_API*>(manager.RetrieveAPI(RENDERER_CONTEXT_API_ID));
    Plugin_API* renderer_core_api = reinterpret_cast<Plugin_API*>(manager.RetrieveBaseAPI(RENDERER_CONTEXT_API_ID));
    ResourceContext_API* resource_api = reinterpret_cast<ResourceContext_API*>(manager.RetrieveAPI(RESOURCE_CONTEXT_API_ID));
    context = renderer_api->GetContext();

    data = StartingData {
        renderer_api,
        resource_api,
        ""
    };

    auto& scene = TetherSimScene::GetScene();
    scene.Construct(RequiredVprObjects{ context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain }, &data);

    SwapchainCallbacks_API callbacks{ nullptr };
    callbacks.BeginSwapchainResize = BeginResizeCallback;
    callbacks.CompleteSwapchainResize = CompleteResizeCallback;
    renderer_api->RegisterSwapchainCallbacks(&callbacks);

    auto& gui = ImGuiWrapper::GetImGuiWrapper();

    while (!renderer_api->WindowShouldClose()) {
        renderer_core_api->LogicalUpdate();
        gui.NewFrame();
        ImGui::ShowDemoWindow();
        gui.EndImGuiFrame();
        resource_api->CompletePendingTransfers();
        scene.Render(nullptr);
    }
}
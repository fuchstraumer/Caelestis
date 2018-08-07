#include "VulkanTriangle.hpp"
#include "../../../plugins/renderer_context/include/RendererContextAPI.hpp"
#include "../../../plugins//renderer_context/include/core/RendererContext.hpp"
#include "../../../plugin_manager/include/PluginManager.hpp"
#include "../../..//plugin_manager/include/CoreAPIs.hpp"

static RendererContext* context = nullptr;

static void BeginRecreateCallback(uint64_t handle, uint32_t width, uint32_t height) {
    auto& tri = VulkanTriangle::GetScene();
    tri.Destroy();
}

static void CompleteResizeCallback(uint64_t handle, uint32_t width, uint32_t height) {
    auto& tri = VulkanTriangle::GetScene();
    RequiredVprObjects objects{
        context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain
    };
    tri.Construct(objects, nullptr);
}

int main(int argc, char* argv[]) {
    PluginManager& manager = PluginManager::GetPluginManager();
    manager.LoadPlugin("application_context.dll");
    manager.LoadPlugin("renderer_context.dll");
    RendererContext_API* renderer_api = reinterpret_cast<RendererContext_API*>(manager.RetrieveAPI(RENDERER_CONTEXT_API_ID));
    context = renderer_api->GetContext();

    SwapchainCallbacks_API callbacks{ nullptr };
    callbacks.BeginSwapchainResize = BeginRecreateCallback;
    callbacks.CompleteSwapchainResize = CompleteResizeCallback;
    renderer_api->RegisterSwapchainCallbacks(&callbacks);

    auto& triangle = VulkanTriangle::GetScene();
    RequiredVprObjects objects{
        context->LogicalDevice, context->PhysicalDevices[0], context->VulkanInstance, context->Swapchain
    };
    triangle.Construct(objects, nullptr);

    Plugin_API* core_api = reinterpret_cast<Plugin_API*>(manager.RetrieveBaseAPI(RENDERER_CONTEXT_API_ID));

    while (!renderer_api->WindowShouldClose()) {
        core_api->LogicalUpdate();
        triangle.Render(nullptr);
    }

}

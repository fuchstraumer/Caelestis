#include "RendererContextAPI.hpp"
#include "PluginAPI.hpp"
#include "CoreAPIs.hpp"
#include "../../application_context/include/AppContextAPI.hpp"
#include "core/RendererContext.hpp"
#include "core/PlatformWindow.hpp"
#include "core/WindowInput.hpp"
#include "vpr/Allocator.hpp"
#include "vpr/LogicalDevice.hpp"
#include "vpr/PhysicalDevice.hpp"
#include "vpr/Instance.hpp"
#include "vpr/Swapchain.hpp"
#include "GLFW/glfw3.h"
#include <vector>
#include <memory>
#include <list>
#include <functional>

static ApplicationContext_API* AppContextAPI = nullptr;
static RendererContext* Context = nullptr;

struct SwapchainCallbackLists {
    using SwapchainCallbackDefaultType = std::add_pointer<void(uint32_t,uint32_t,uint32_t)>::type;
    using SwapchainDestructionCallbackType = std::add_pointer<void(uint32_t)>::type;
    std::list<SwapchainCallbackDefaultType> CreationCallbacks;
    std::list<SwapchainCallbackDefaultType> BeginResizeCallbacks;
    std::list<SwapchainCallbackDefaultType> CompleteResizeCallbacks;
    std::list<SwapchainDestructionCallbackType> DestructionCallbacks;
};

static SwapchainCallbackLists SwapchainCallbacks;

#pragma warning(push)
#pragma warning(disable: 4302)
#pragma warning(disable: 4311)
static void RecreateSwapchain() {

    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(Context->Window->glfwWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(Context->LogicalDevice->vkHandle());

    Context->Window->GetWindowSize(width, height);

    for (auto& fn : SwapchainCallbacks.BeginResizeCallbacks) {
        fn((uint32_t)(Context->Swapchain->vkHandle()), width, height);
    }

    vpr::RecreateSwapchainAndSurface(Context->LogicalDevice, Context->VulkanInstance, Context->Swapchain);

    Context->Window->GetWindowSize(width, height);
    for (auto& fn : SwapchainCallbacks.CompleteResizeCallbacks) {
        fn((uint32_t)(Context->Swapchain->vkHandle()), width, height);
    }

    vkDeviceWaitIdle(Context->LogicalDevice->vkHandle());
}
#pragma warning(pop)

static uint32_t GetID() {
    return RENDERER_CONTEXT_API_ID;
}

static const char* GetName() {
    return "RendererContext";
}

static void Load(GetEngineAPI_Fn fn) {
    AppContextAPI = reinterpret_cast<ApplicationContext_API*>(fn(APPLICATION_CONTEXT_API_ID));
    const char* cfg_file = AppContextAPI->GetModuleConfigFile(GetName());
    Context = new RendererContext(cfg_file, AppContextAPI);
}

static void Unload() {
    if (Context) {
        delete Context;
    }
}

static void LogicalUpdate() {
    Context->Window->GetWindowInput()->Update();
    if (RendererContext::ShouldResizeExchange(false)) {
        RecreateSwapchain();
    }
}

static void Update(float dt) {

}

static RendererContext* GetContext() {
    return Context;
}

static void AddSwapchainCallbacks(SwapchainCallbacks_API* api) {
    if (api->SwapchainCreated) {
        SwapchainCallbacks.CreationCallbacks.push_back(api->SwapchainCreated);
    }
    if (api->BeginSwapchainResize) {
        SwapchainCallbacks.BeginResizeCallbacks.push_back(api->BeginSwapchainResize);
    }
    if (api->CompleteSwapchainResize) {
        SwapchainCallbacks.CompleteResizeCallbacks.push_back(api->CompleteSwapchainResize);
    }
    if (api->SwapchainDestroyed) {
        SwapchainCallbacks.DestructionCallbacks.push_back(api->SwapchainDestroyed);
    }
}

static void SetWindowUserPointer(void* user_ptr) {
    Context->Window->SetWindowUserPointer(user_ptr);
}

static void* getGlfwWindow() {
    return Context->Window->glfwWindow();
}

static void GetWindowSize(int& width, int& height) {
    Context->Window->GetWindowSize(width, height);
}

static WindowInput* GetWindowInput() {
    return GetContext()->Window->GetWindowInput();
}

static void AddMonitoredScancode(int code) {
    GetWindowInput()->AddMonitoredScancode(code);
}

static void RemoveMonitoredScancode(int code) {
    GetWindowInput()->RemoveMonitoredScancode(code);
}

static void GetScancodeActions(int code, int* actions, int* modifiers) {
    GetWindowInput()->GetScancodeActions(code, actions, modifiers);
}

static void ConsumeScancodeActions(int code, int* actions, int* modifiers) {
    GetWindowInput()->ConsumeMouseAction(code, actions, modifiers);
}

static void FlushScancodeActions() {
    GetWindowInput()->FlushScancodeActions();
}

static VkDevice GetDeviceHandle() {
    return Context->LogicalDevice->vkHandle();
}


static Plugin_API* CoreAPI() {
    static Plugin_API api{ nullptr };
    api.PluginID = GetID;
    api.PluginName = GetName;
    api.Load = Load;
    api.Unload = Unload;
    api.LogicalUpdate = LogicalUpdate;
    api.TimeDependentUpdate = Update;
    return &api;
}

static RendererContext_API* RendererContextAPI() {
    static RendererContext_API api{ nullptr };
    api.GetContext = GetContext;
    api.RegisterSwapchainCallbacks = AddSwapchainCallbacks;
    api.SetWindowUserPointer = SetWindowUserPointer;
    api.GetGLFWwindow = getGlfwWindow;
    api.GetWindowSize = GetWindowSize;
    api.AddMonitoredScancode = AddMonitoredScancode;
    api.RemoveMonitoredScancode = RemoveMonitoredScancode;
    api.GetScancodeActions = GetScancodeActions;
    api.ConsumeScancodeActions = ConsumeScancodeActions;
    api.FlushScancodeActions = FlushScancodeActions;
    return &api;
}

PLUGIN_API void* GetPluginAPI(uint32_t api_id) {
    switch (api_id) {
    case 0:
        return CoreAPI();
    case RENDERER_CONTEXT_API_ID:
        return RendererContextAPI();
    default:
        return nullptr;
    };
}
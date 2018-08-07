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
#include "vpr/SurfaceKHR.hpp"
#include "vpr/PipelineCache.hpp"
#include "GLFW/glfw3.h"
#include <vector>
#include <memory>
#include <list>
#include <forward_list>
#include <functional>
#include <string>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>   // for glfwGetWin32Window
#endif
static ApplicationContext_API* AppContextAPI = nullptr;
static RendererContext* Context = nullptr;

struct SwapchainCallbackLists {
    using SwapchainCallbackDefaultType = std::add_pointer<void(uint64_t,uint32_t,uint32_t)>::type;
    using SwapchainDestructionCallbackType = std::add_pointer<void(uint64_t)>::type;
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
        fn((uint64_t)(Context->Swapchain->vkHandle()), width, height);
    }

    vpr::RecreateSwapchainAndSurface(Context->Swapchain, Context->WindowSurface);
    Context->LogicalDevice->UpdateSurface(Context->WindowSurface->vkHandle());

    Context->Window->GetWindowSize(width, height);
    for (auto& fn : SwapchainCallbacks.CompleteResizeCallbacks) {
        fn((uint64_t)(Context->Swapchain->vkHandle()), width, height);
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

static void WriteLoadedInfoLog() {
    AppContextAPI->InfoLog("Loaded RendererContext");
    int w = 0; int h = 0;
    Context->Window->GetWindowSize(w, h);
    std::string window_info_str = "Created window with dimensions " + std::to_string(w) + " , " + std::to_string(h) + " in window mode " + Context->WindowMode;
    AppContextAPI->InfoLog(window_info_str.c_str());
    if (Context->NumInstanceExtensions != 0) {
        AppContextAPI->InfoLog("Enabled Instance Extensions: ");
        for (size_t i = 0; i < Context->NumInstanceExtensions; ++i) {
            std::string extension_str("    " + std::string(Context->EnabledInstanceExtensions[i]));
            AppContextAPI->InfoLog(extension_str.c_str());
        }
    }
    if (Context->NumDeviceExtensions != 0) {
        AppContextAPI->InfoLog("Enabled Device Extensions: ");
        for (size_t i = 0; i < Context->NumDeviceExtensions; ++i) {
            std::string extension_str("    " + std::string(Context->EnabledDeviceExtensions[i]));
            AppContextAPI->InfoLog(extension_str.c_str());
        }
    }
}

static void Load(GetEngineAPI_Fn fn) {
    AppContextAPI = reinterpret_cast<ApplicationContext_API*>(fn(APPLICATION_CONTEXT_API_ID));
    void* logging_storage_ptr = AppContextAPI->GetLoggingStoragePointer();
    vpr::SetLoggingRepository_VprCore(logging_storage_ptr);
    const char* cfg_file = AppContextAPI->GetPluginConfigFile(GetName());
    Context = new RendererContext(cfg_file, AppContextAPI);
    WriteLoadedInfoLog();
}

static void Unload() {
    if (Context) {
        delete Context;
    }
}

static void LogicalUpdate() {
    Context->Window->Update();
    if (RendererContext::ShouldResizeExchange(false)) {
        RecreateSwapchain();
    }
}

static void Update(double dt) {

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

static void* getGlfwWindow() {
    return Context->Window->glfwWindow();
}

static void GetWindowSize(int& width, int& height) {
    Context->Window->GetWindowSize(width, height);
}

static void GetFramebufferSize(int& w, int& h) {
    glfwGetFramebufferSize(Context->Window->glfwWindow(), &w, &h);
}

static const char* GetClipboardText() {
    return glfwGetClipboardString(Context->Window->glfwWindow());
}

static bool MouseLockingEnabled() {
    int mode = glfwGetInputMode(Context->Window->glfwWindow(), GLFW_CURSOR);
    return mode == GLFW_CURSOR_DISABLED;
}

static void SetInputMode(int mode, int value) {
    Context->Window->SetInputMode(mode, value);
}

static int GetInputMode(int mode) {
    return glfwGetInputMode(Context->Window->glfwWindow(), mode);
}

static void SetCursorPosition(double x, double y) {
    glfwSetCursorPos(Context->Window->glfwWindow(), x, y);
}

static void SetCursor(void* cursor) {
    glfwSetCursor(Context->Window->glfwWindow(), reinterpret_cast<GLFWcursor*>(cursor));
}

static void* CreateCursor(void* glfw_image, int w, int h) {
    return glfwCreateCursor(reinterpret_cast<GLFWimage*>(glfw_image), w, h);
}

static void* CreateStandardCursor(int type) {
    return glfwCreateStandardCursor(type);
}

static void DestroyCursor(void* cursor) {
    glfwDestroyCursor(reinterpret_cast<GLFWcursor*>(cursor));
}

static void GetCursorPosition(double& x, double& y) {
    glfwGetCursorPos(Context->Window->glfwWindow(), &x, &y);
}

static bool ShouldWindowClose() {
    return Context->Window->WindowShouldClose();
}

static void RegisterCursorPos(cursor_pos_callback_t pos_fn) {
    Context->Window->AddCursorPosCallbackFn(pos_fn);
}

static void RegisterCursorEnter(cursor_enter_callback_t fn) {
    Context->Window->AddCursorEnterCallbackFn(fn);
}

static void RegisterScrollFn(scroll_callback_t fn) {
    Context->Window->AddScrollCallbackFn(fn);
}

static void RegisterCharFn(char_callback_t fn) {
    Context->Window->AddCharCallbackFn(fn);
}

static void RegisterPathDropFn(path_drop_callback_t fn) {
    Context->Window->AddPathDropCallbackFn(fn);
}

static void RegisterMouseButtonFn(mouse_button_callback_t fn) {
    Context->Window->AddMouseButtonCallbackFn(fn);
}

static void RegisterKeyboardFn(keyboard_key_callback_t fn) {
    Context->Window->AddKeyboardKeyCallbackFn(fn);
}

static void* GetWin32_WindowHandle() {
#ifdef _WIN32
    return glfwGetWin32Window(Context->Window->glfwWindow());
#else
    return nullptr;
#endif
}

static int GetMouseButton(int button) {
    return glfwGetMouseButton(Context->Window->glfwWindow(), button);
}

static int GetWindowAttribute(int attrib) {
    return glfwGetWindowAttrib(Context->Window->glfwWindow(), attrib);
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
    api.GetWindowSize = GetWindowSize;
    api.GetFramebufferSize = GetFramebufferSize;
    api.GetClipboardText = GetClipboardText;
    api.MouseLockingEnabled = MouseLockingEnabled;
    api.SetInputMode = SetInputMode;
    api.GetInputMode = GetInputMode;
    api.GetCursorPosition = GetCursorPosition;
    api.SetCursorPosition = SetCursorPosition;
    api.SetCursor = SetCursor;
    api.CreateCursor = CreateCursor;
    api.CreateStandardCursor = CreateStandardCursor;
    api.DestroyCursor = DestroyCursor;
    api.WindowShouldClose = ShouldWindowClose;
    api.RegisterCursorPosCallback = RegisterCursorPos;
    api.RegisterCursorEnterCallback = RegisterCursorEnter;
    api.RegisterScrollCallback = RegisterScrollFn;
    api.RegisterCharCallback = RegisterCharFn;
    api.RegisterPathDropCallback = RegisterPathDropFn;
    api.RegisterMouseButtonCallback = RegisterMouseButtonFn;
    api.RegisterKeyboardKeyCallback = RegisterKeyboardFn;
    api.GetWin32_WindowHandle = GetWin32_WindowHandle;
    api.GetMouseButton = GetMouseButton;
    api.GetWindowAttribute = GetWindowAttribute;
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
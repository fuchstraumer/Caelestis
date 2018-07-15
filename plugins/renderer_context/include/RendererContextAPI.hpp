#pragma once
#ifndef RENDERER_CONTEXT_API_HPP
#define RENDERER_CONTEXT_API_HPP
#include <cstdint>

constexpr static uint32_t RENDERER_CONTEXT_API_ID = 0x100c217e;
constexpr static uint32_t SWAPCHAIN_CALLBACKS_API_ID = 0x73e0a080;

/*
    Implement this API in plugins that need to be notified on
    swapchain events. Will be registered and added to an internal
    list of callbacks to traverse, in this plugin
*/

struct SwapchainCallbacks_API {
    void (*SwapchainCreated)(uint32_t swapchain_handle, uint32_t width, uint32_t height);
    // Occurs upon first receipt of swapchain resize event. Signals that appropriate resources should be destroyed
    void (*BeginSwapchainResize)(uint32_t swapchain_handle, uint32_t width, uint32_t height);
    // Occurs once swapchain has been recreated and updated succesfully: recreate destroyed resources as appropriate.
    void (*CompleteSwapchainResize)(uint32_t swapchain_handle, uint32_t width, uint32_t height);
    void (*SwapchainDestroyed)(uint32_t swapchain_handle);
};

struct RendererContext_API {
    struct RendererContext* (*GetContext)();
    void (*RegisterSwapchainCallbacks)(SwapchainCallbacks_API* callbacks);
    bool (*DeviceHasExtension)(const char* name);
    // Interface to platform window
    void (*SetWindowUserPointer)(void* user_ptr);
    void*(*GetGLFWwindow)(void);
    void (*GetWindowSize)(int& width, int& height);
    // Input system interface
    void (*AddMonitoredScancode)(int code);
    void (*RemoveMonitoredScancode)(int code);
    void (*GetScancodeActions)(int code, int* actions, int* modifiers);
    void (*ConsumeScancodeActions)(int code, int* actions, int* modifiers);
    void (*FlushScancodeActions)(void);
    void (*GetInputCharacters)(size_t* num_chars, unsigned int* read_chars);
    void (*GetMouseActions)(int button, int* actions, int* modifiers);
    void (*ConsumeMouseAction)(int button, int* actions, int* modifiers);
    const char* (*GetClipboardText)(void);
};

#endif //!RENDERER_CONTEXT_API_HPP

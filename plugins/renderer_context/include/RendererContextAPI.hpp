#pragma once
#ifndef RENDERER_CONTEXT_API_HPP
#define RENDERER_CONTEXT_API_HPP
#include <cstdint>

constexpr static uint32_t RENDERER_CONTEXT_API_ID = 0x100c217e;
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
    // Interface to platform window
    void (*GetWindowSize)(int& width, int& height);
    // Input handling interface
    const char* (*GetClipboardText)(void);
    // Affected by input mode. GLFW_CURSOR_DISABLED -> locking is enabled
    bool(*MouseLockingEnabled)(void);
    // Set input mode - mode being one of the following from the glfw header:
    // GLFW_CURSOR_NORMAL - cursor is free and visible on the screen. position bounded.
    // GLFW_CURSOR_HIDDEN - cursor is free, but not visible on the screen. position bounded.
    // GLFW_CURSOR_DISABLED - cursor is "locked" but position is unbounded
    void (*SetInputMode)(int mode);
    void (*GetCursorPosition)(double& x_pos, double& y_pos);
    bool (*WindowShouldClose)(void);
    using cursor_pos_callback_t = void(*)(double pos_x, double pos_y);
    using cursor_enter_callback_t = void(*)(int enter);
    using scroll_callback_t = void(*)(double scroll_x, double scroll_y);
    using char_callback_t = void(*)(unsigned int code_point);
    using path_drop_callback_t = void(*)(int count, const char** paths);
    using mouse_button_callback_t = void(*)(int button, int action, int mods);
    using keyboard_key_callback_t = void(*)(int key, int scancode, int action, int mods);
    void (*RegisterCursorPosCallback)(cursor_pos_callback_t callback_fn);
    void (*RegisterCursorEnterCallback)(cursor_enter_callback_t callback_fn);
    void (*RegisterScrollCallback)(scroll_callback_t callback_fn);
    void (*RegisterCharCallback)(char_callback_t callback_fn);
    void (*RegisterPathDropCallback)(path_drop_callback_t callback_fn);
    void (*RegisterMouseButtonCallback)(mouse_button_callback_t callback_fn);
    void (*RegisterKeyboardKeyCallback)(keyboard_key_callback_t callback_fn);
};

#endif //!RENDERER_CONTEXT_API_HPP

#pragma once
#ifndef RENDERER_CONTEXT_PLATFORM_WINDOW_HPP
#define RENDERER_CONTEXT_PLATFORM_WINDOW_HPP
#include <cstdint>

class WindowInput;
struct GLFWwindow;

namespace windowing_modes {
    enum e : uint32_t {
        None = 0,
        Fullscreen = 1,
        BorderlessWindowed = 2,
        Windowed = 3
    };
}

struct WindowCallbackLists;
using cursor_pos_callback_t = void(*)(double pos_x, double pos_y);
using cursor_enter_callback_t = void(*)(int enter);
using scroll_callback_t = void(*)(double scroll_x, double scroll_y);
using char_callback_t = void(*)(unsigned int code_point);
using path_drop_callback_t = void(*)(int count, const char** paths);
using mouse_button_callback_t = void(*)(int button, int action, int mods);
using keyboard_key_callback_t = void(*)(int key, int scancode, int action, int mods);

class PlatformWindow {
    PlatformWindow(const PlatformWindow&) = delete;
    PlatformWindow& operator=(const PlatformWindow&) = delete;
public:

    PlatformWindow(int width, int height, const char* app_name, uint32_t window_mode);
    ~PlatformWindow();

    void SetWindowUserPointer(void* user_ptr);
    GLFWwindow* glfwWindow() noexcept;
    void GetWindowSize(int& w, int& h) noexcept;
    void Update();
    void WaitForEvents();
    bool WindowShouldClose();

    void AddCursorPosCallbackFn(cursor_pos_callback_t fn);
    void AddCursorEnterCallbackFn(cursor_enter_callback_t fn);
    void AddScrollCallbackFn(scroll_callback_t fn);
    void AddCharCallbackFn(char_callback_t fn);
    void AddPathDropCallbackFn(path_drop_callback_t fn);
    void AddMouseButtonCallbackFn(mouse_button_callback_t fn);
    void AddKeyboardKeyCallbackFn(keyboard_key_callback_t fn);
    void SetCursorInputMode(int mode);

    WindowCallbackLists& GetCallbacks();

private:

    void createWindow(const char* app_name);
    void setCallbacks();
    GLFWwindow* window;
    int width;
    int height;
    WindowCallbackLists* callbacks;
    uint32_t windowMode;
};


#endif //!RENDERER_CONTEXT_PLATFORM_WINDOW_HPP

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

class PlatformWindow {
    PlatformWindow(const PlatformWindow&) = delete;
    PlatformWindow& operator=(const PlatformWindow&) = delete;
public:

    PlatformWindow(int width, int height, const char* app_name, uint32_t window_mode);
    ~PlatformWindow();

    void SetWindowUserPointer(void* user_ptr);
    GLFWwindow* glfwWindow() noexcept;
    void GetWindowSize(int& w, int& h) noexcept;
    WindowInput* GetWindowInput() noexcept;

private:

    void createWindow(const char* app_name);
    void createInputHandler();

    GLFWwindow* window;
    int width;
    int height;
    WindowInput* inputManager;
    uint32_t windowMode;
};


#endif //!RENDERER_CONTEXT_PLATFORM_WINDOW_HPP

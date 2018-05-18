#pragma once
#ifndef VPSK_PLATFORM_WINDOW_HPP
#define VPSK_PLATFORM_WINDOW_HPP
#include "GLFW/glfw3.h"
#include <memory>
namespace vpsk {

    class InputHandler;

    class PlatformWindow {
        PlatformWindow(const PlatformWindow&) = delete;
        PlatformWindow& operator=(const PlatformWindow&) = delete;
    public:

        PlatformWindow(const int& width, const int& height, const char* app_name, const bool& fullscreen);
        ~PlatformWindow();

        void SetWindowUserPointer(void* user_ptr);
        GLFWwindow* glfwWindow() noexcept;
        void GetWindowSize(int& width, int& height) const noexcept;

        static void ResizeCallback(GLFWwindow* window, int width, int height);

        InputHandler& GetInputHandler() noexcept;

    private: 
        std::unique_ptr<InputHandler> inputHandler;
        void createWindow(const char* app_name);
        void createInputHandler();
        GLFWwindow* window;
        int width, height;
        bool fullscreen;
    };

}

#endif // !VPSK_PLATFORM_WINDOW_HPP

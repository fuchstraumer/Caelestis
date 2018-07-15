#include "core/PlatformWindow.hpp"
#include "core/WindowInput.hpp"
#define GLFW_INCLUDE_VULKAN
#if defined(_WIN32) 
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#endif
#include "core/RendererContext.hpp"

static void ResizeCallback(GLFWwindow* window, int width, int height) {
    RendererContext::SetShouldResize(true);
}

PlatformWindow::PlatformWindow(int w, int h, const char* app_name, uint32_t window_mode) : width(w), height(h), windowMode(window_mode) {
    createWindow(app_name);
    glfwSetWindowSizeCallback(window, ResizeCallback);
    inputManager = new WindowInput(window);
}

PlatformWindow::~PlatformWindow() {
    if (inputManager) {
        delete inputManager;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

void PlatformWindow::SetWindowUserPointer(void* user_ptr) {
    glfwSetWindowUserPointer(window, user_ptr);
}

GLFWwindow* PlatformWindow::glfwWindow() noexcept {
    return window;
}

void PlatformWindow::GetWindowSize(int& w, int& h) noexcept {
    glfwGetWindowSize(window, &width, &height);
    w = width;
    h = height;
}

WindowInput* PlatformWindow::GetWindowInput() noexcept {
    return inputManager;
}

void PlatformWindow::createWindow(const char* name) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (windowMode == windowing_modes::Fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, vidmode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, vidmode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, vidmode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, vidmode->refreshRate);
        width = vidmode->width;
        height = vidmode->height;
        window = glfwCreateWindow(width, height, name, monitor, nullptr);
    }
    else if (windowMode == windowing_modes::BorderlessWindowed) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    }
    else {
        window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    }
}
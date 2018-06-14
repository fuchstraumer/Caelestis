#include "PlatformWindow.hpp"
#include "InputHandler.hpp"
#include "RenderingContext.hpp"
#include "imgui/imgui.h"
#if defined(_WIN32) 
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "GLFW/glfw3native.h"
#endif
namespace vpsk {

    PlatformWindow::PlatformWindow(const int & _width, const int & _height, const char * app_name, const bool & _fullscreen) : width(_width), height(_height), fullscreen(_fullscreen) {
        createWindow(app_name);
        createInputHandler();
    }

    PlatformWindow::~PlatformWindow() {
        inputHandler.reset();
    }

    void PlatformWindow::SetWindowUserPointer(void * user_ptr) {
        glfwSetWindowUserPointer(window, user_ptr);
    }

    GLFWwindow * PlatformWindow::glfwWindow() noexcept {
        return window;
    }

    void PlatformWindow::GetWindowSize(int & _width, int & _height) const noexcept {
        _width = width;
        _height = height;
    }

    void PlatformWindow::ResizeCallback(GLFWwindow * window, int width, int height) {
        ImGuiIO& io = ImGui::GetIO();
        if (width == 0 || height == 0) {
            return;
        }
        else {
            io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        }
        // Flag receipt of resize event.
        RenderingContext::ShouldResize() = true;
    }

    InputHandler & PlatformWindow::GetInputHandler() noexcept {
        return *inputHandler;
    }

    void PlatformWindow::createWindow(const char * app_name) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        if (!fullscreen) {
            window = glfwCreateWindow(width, height, app_name, nullptr, nullptr);
        }
        else {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, vidmode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, vidmode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, vidmode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, vidmode->refreshRate);
            window = glfwCreateWindow(width, height, app_name, monitor, nullptr);
            width = vidmode->width;
            height = vidmode->height;
        }

        ImGuiIO& io = ImGui::GetIO();
#ifdef _WIN32
        io.ImeWindowHandle = glfwGetWin32Window(window);
#endif
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        glfwSetWindowSizeCallback(window, ResizeCallback);
    }

    void PlatformWindow::createInputHandler() {
        inputHandler = std::make_unique<InputHandler>(this);
    }

}

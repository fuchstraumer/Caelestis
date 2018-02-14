#include "scene/Window.hpp"
#include "scene/InputHandler.hpp"
#include "scene/BaseScene.hpp"
#include <imgui/imgui.h>
#if defined(_WIN32) 
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "GLFW/glfw3native.h"
#endif
namespace vpsk {

    Window::Window(const uint32_t& _width, const uint32_t& _height, const std::string& app_name, const bool& fullscreen) : width(_width), height(_height) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        createWindow(app_name, fullscreen);
    }

    void Window::createWindow(const std::string& app_name, const bool& fullscreen) {

        if (!fullscreen) {
            window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), app_name.c_str(), nullptr, nullptr);
        }
        else {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            window = glfwCreateWindow(mode->width, mode->height, app_name.c_str(), monitor, nullptr);
        }
        ImGuiIO& io = ImGui::GetIO();
#ifdef _WIN32
        io.ImeWindowHandle = glfwGetWin32Window(window);
#endif
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

        glfwSetWindowSizeCallback(window, ResizeCallback);
        createInputHandler();

    }

    GLFWwindow* Window::glfwWindow() noexcept {
        return window;
    }

    glm::ivec2 Window::GetWindowSize() const noexcept{
        glm::ivec2 result(0, 0);
        glfwGetWindowSize(const_cast<GLFWwindow*>(window), &result.x, &result.y);
        return result;
    }

    void Window::createInputHandler() {
        InputHandler = std::make_unique<input_handler>(this);
    }

    void Window::ResizeCallback(GLFWwindow* window, int width, int height) {

        ImGuiIO& io = ImGui::GetIO();
        if(width == 0 || height == 0) {
            width = static_cast<int>(io.DisplaySize.x);
            height = static_cast<int>(io.DisplaySize.y);        
        }
        else {
            io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        }

        BaseScene::ShouldResize = true;

    }

    void Window::SetWindowUserPointer(void* user_ptr) {
        glfwSetWindowUserPointer(window, user_ptr);
    }

}
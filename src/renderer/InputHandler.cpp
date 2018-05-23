#include "renderer/InputHandler.hpp"
#include "renderer/PlatformWindow.hpp"
#include "imgui/imgui.h"
namespace vpsk {

    std::array<bool, 1024> InputHandler::Keys = std::array<bool, 1024>();
    std::array<bool, 12> InputHandler::MouseButtons = std::array<bool, 12>();
    float InputHandler::LastMouseX = 0.0f;
    float InputHandler::LastMouseY = 0.0f;
    float InputHandler::MouseDx = 0.0f;
    float InputHandler::MouseDy = 0.0f;
    float InputHandler::MouseScroll = 0.0f;

    InputHandler::InputHandler(PlatformWindow * _parent) : parent(_parent) {
        setCallbacks();
        setImguiMapping();
    }

    void InputHandler::setCallbacks() {
        glfwSetCursorPosCallback(parent->glfwWindow(), MousePositionCallback);
        glfwSetKeyCallback(parent->glfwWindow(), KeyboardCallback);
        glfwSetMouseButtonCallback(parent->glfwWindow(), MouseButtonCallback);
        glfwSetScrollCallback(parent->glfwWindow(), MouseScrollCallback);
        glfwSetCharCallback(parent->glfwWindow(), CharCallback);
    }

    void InputHandler::setImguiMapping() const {
        ImGuiIO& io = ImGui::GetIO();
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
        io.SetClipboardTextFn = SetClipboardStrCallback;
        io.GetClipboardTextFn = GetClipboardStrCallback;
    }

    void InputHandler::MousePositionCallback(GLFWwindow * window, double mouse_x, double mouse_y) {
        MouseDx = static_cast<float>(mouse_x) - LastMouseX;
        MouseDy = LastMouseY - static_cast<float>(mouse_y);

        LastMouseX = static_cast<float>(mouse_x);
        LastMouseY = static_cast<float>(mouse_y);

        ImGuiIO& io = ImGui::GetIO();
        io.MousePos.x = LastMouseX;
        io.MousePos.y = LastMouseY;

    }

    void InputHandler::MouseButtonCallback(GLFWwindow * window, int button, int action, int code) {

        ImGuiIO& io = ImGui::GetIO();

        if (button >= 0 && button < 3) {
            if (action == GLFW_PRESS) {
                MouseButtons[button] = true;
                io.MouseDown[button] = true;
            }
            else if (action == GLFW_RELEASE) {
                MouseButtons[button] = false;
                io.MouseDown[button] = false;
            }
        }
    }

    void InputHandler::MouseScrollCallback(GLFWwindow * window, double x_offset, double y_offset) {

        MouseScroll += static_cast<float>(y_offset);
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheel = static_cast<float>(y_offset);

    }

    void InputHandler::KeyboardCallback(GLFWwindow * window, int key, int scan_code, int action, int mods) {

        auto io = ImGui::GetIO();

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        if (key >= 0 && key < 1024) {
            if (action == GLFW_PRESS) {
                Keys[key] = true;
                io.KeysDown[key] = true;
            }
            if (action == GLFW_RELEASE) {
                Keys[key] = false;
                io.KeysDown[key] = false;
            }
        }

    }

    void InputHandler::CharCallback(GLFWwindow*, unsigned int c) {
        ImGuiIO& io = ImGui::GetIO();
        if (c > 0 && c < 0x10000) {
            io.AddInputCharacter(static_cast<unsigned short>(c));
        }
    }

    void InputHandler::SetClipboardStrCallback(void * window, const char * text) {
        glfwSetClipboardString(reinterpret_cast<GLFWwindow*>(window), text);
    }

    const char* InputHandler::GetClipboardStrCallback(void* window) {
        return glfwGetClipboardString(reinterpret_cast<GLFWwindow*>(window));
    }

}

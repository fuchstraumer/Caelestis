#pragma once
#ifndef VPSK_INPUT_HANDLER_HPP
#define VPSK_INPUT_HANDLER_HPP
#include <array>
struct GLFWwindow;

namespace vpsk {
    
    class PlatformWindow;

    class InputHandler {
        InputHandler(const InputHandler&) = delete;
        InputHandler& operator=(const InputHandler&) = delete;
    public:
        InputHandler(PlatformWindow* _parent);

        static void MousePositionCallback(GLFWwindow* window, double mouse_x, double mouse_y);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int code);
        static void MouseScrollCallback(GLFWwindow* window, double x_offset, double y_offset);
        static void KeyboardCallback(GLFWwindow* window, int key, int scan_code, int action, int mods);
        static void CharCallback(GLFWwindow *, unsigned int c);
        static void SetClipboardStrCallback(void* window, const char* text);
        static const char* GetClipboardStrCallback(void* window);

        static std::array<bool, 1024> Keys;
        static std::array<bool, 12> MouseButtons;

        static float LastMouseX;
        static float LastMouseY;
        static float MouseDx;
        static float MouseDy;
        static float MouseScroll;

    private:
        void setImguiMapping() const;
        void setCallbacks();
        PlatformWindow* parent;
    };

}

#endif // !VPSK_INPUT_HANDLER_HPP

#pragma once
#ifndef VULPES_VK_WINDOW_H
#define VULPES_VK_WINDOW_H
#include "ForwardDecl.hpp"
#include <cstdint>
#include <string>
#include <memory>
#include <glm/vec2.hpp>
#include <GLFW/glfw3.h>

namespace vpsk {

    struct input_handler;

    /*! Window is a wrapper around the GLFW windowing system, and handles creating the underlying rendering
    *    window along with creating a suitable VkSurfaceKHR object. It is also responsible for signaling a
    *    window resizing event.
    * \ingroup Core
    */
    class Window {
        Window(const Window& other) = delete;
        Window& operator=(const Window& other) = delete;
    public:

        Window(const uint32_t& width, const uint32_t& height, const std::string& app_name, const bool& fullscreen = false);

        /** !This method attaches any object - usually a scene of some sort - to this window, allowing this class to signal the attached object 
        *   that a window resize or window mode change has occured. This is done to the BaseScene class in the examples for this project.
        */
        void SetWindowUserPointer(void* user_ptr);
        GLFWwindow* glfwWindow() noexcept;
        glm::ivec2 GetWindowSize() const noexcept;

        /**! This method is called when GLFW detects a window resize or window mode change. It is within this method that the object that is pointed
        *    to by the SetWindowUserPointer method is accessed. In this case, the BaseScene class has it's RecreateSwapchain method called.
        *    \todo This method really needs to be generalized, or somehow overridable so that it doesn't call the BaseScene class in case this class is not being used.
        */
        static void ResizeCallback(GLFWwindow* window, int width, int height);

        std::unique_ptr<input_handler> InputHandler;
    private:

        void createWindow(const std::string& app_name, const bool& fullscreen);
        void createInputHandler();    
        GLFWwindow* window;
        uint32_t width, height;

    };
}

#endif //!VULPES_VK_WINDOW_H
#pragma once
#ifndef RENDERER_CONTEXT_OBJECT_HPP
#define RENDERER_CONTEXT_OBJECT_HPP
#include <cstddef>

namespace vpr {
    class Instance;
    class PhysicalDevice;
    class Device;
    class Swapchain;
    class SurfaceKHR;
}

class PlatformWindow;

struct RendererContext {
    RendererContext(const char* json_cfg_file, struct ApplicationContext_API* app_context_api);
    ~RendererContext();
    static void SetShouldResize(const bool& resize);
    static bool ShouldResizeExchange(bool value);
    vpr::Instance* VulkanInstance{ nullptr };
    vpr::PhysicalDevice** PhysicalDevices{ nullptr };
    size_t NumPhysicalDevices{ 0 };
    vpr::Device* LogicalDevice{ nullptr };
    vpr::Swapchain* Swapchain{ nullptr };
    size_t NumVirtualSwapchains{ 0 };
    vpr::Swapchain** VirtualSwapchains{ nullptr };
    PlatformWindow* Window{ nullptr };
    vpr::SurfaceKHR* WindowSurface{ nullptr };
    char** EnabledInstanceExtensions{ nullptr };
    size_t NumInstanceExtensions{ 0 };
    char** EnabledDeviceExtensions{ nullptr };
    size_t NumDeviceExtensions{ 0 };
    char** EnabledDeviceFeatures{ nullptr };
    size_t NumFeatures{ 0 };
    const char* WindowMode{ "" };
private:
    RendererContext(const RendererContext&) = delete;
    RendererContext& operator=(const RendererContext&) = delete;
};

#endif //!RENDERER_CONTEXT_OBJECT_HPP

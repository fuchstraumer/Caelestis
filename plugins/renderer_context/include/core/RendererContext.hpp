#pragma once
#ifndef RENDERER_CONTEXT_OBJECT_HPP
#define RENDERER_CONTEXT_OBJECT_HPP

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
    vpr::Instance* VulkanInstance;
    vpr::PhysicalDevice** PhysicalDevices;
    size_t NumPhysicalDevices;
    vpr::Device* LogicalDevice;
    vpr::Swapchain* Swapchain;
    vpr::Swapchain** VirtualSwapchains;
    PlatformWindow* Window;
    vpr::SurfaceKHR* WindowSurface;
    char** EnabledInstanceExtensions;
    size_t NumInstanceExtensions;
    char** EnabledDeviceExtensions;
    size_t NumDeviceExtensions;
    char** EnabledDeviceFeatures;
    size_t NumFeatures;
    const char* WindowMode;
private:
    RendererContext(const RendererContext&) = delete;
    RendererContext& operator=(const RendererContext&) = delete;
};

#endif //!RENDERER_CONTEXT_OBJECT_HPP

#pragma once
#ifndef VPSK_RENDERER_CORE_HPP
#define VPSK_RENDERER_CORE_HPP
#include "ForwardDecl.hpp"
#include <memory>
#include <atomic>
#include <vulkan/vulkan.h>
#include <string>

namespace vpsk {

    // The renderer is a singleton object that encompasses the base objects and systems required
    // for rendering functionality. This includes the main window, the transfer and resource loading systems,
    // the resource pools, and a few other backing objects.
    //
    // New renderers cannot be created, the static singleton renderer can only be retrieved.

    class PlatformWindow;
    class ResourceTransferSystem;

    struct context_configuration_t {
        int Width{ 1440 };
        int Height{ 900 };
        std::string ApplicationName{ "AppName" };
        std::string EngineName{ "VulpesEngine" };
        uint32_t ApplicationVersion{ VK_MAKE_VERSION(0,1,0) };
        uint32_t EngineVersion{ VK_MAKE_VERSION(0,1,0) };
        bool EnableMSAA{ true };
        VkSampleCountFlags MSAA_SampleCount{ VK_SAMPLE_COUNT_4_BIT };
        bool EnableSamplerAnisotropy{ true };
        float SamplerAnisotropy{ 4.0f };
    };

    inline static context_configuration_t ContextConfiguration;

    class RenderingContext {
        RenderingContext();
        ~RenderingContext();
        RenderingContext(const RenderingContext&) = delete;
        RenderingContext& operator=(const RenderingContext&) = delete;
        RenderingContext(RenderingContext&&) = delete;
        RenderingContext& operator=(RenderingContext&&) = delete;
    public:

        static RenderingContext& GetRenderer() noexcept;
        static std::atomic<bool>& ShouldResize() noexcept;
        const vpr::Device* Device() const noexcept;
        vpr::Device* Device() noexcept;
        const vpr::Instance* Instance() const noexcept;
        vpr::Instance* Instance() noexcept;
        const vpr::Swapchain* Swapchain() const noexcept;
        vpr::Swapchain* Swapchain() noexcept;
        ResourceTransferSystem* TransferSystem() noexcept;

    protected:
        std::unique_ptr<ResourceTransferSystem> transferSystem;
        std::unique_ptr<vpr::Instance> instance;
        std::unique_ptr<vpr::Device> device;
        std::unique_ptr<vpr::Swapchain> swapchain;
        std::unique_ptr<PlatformWindow> window;
    };

}

#endif //!VPSK_RENDERER_CORE_HPP
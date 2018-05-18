#include "renderer/RendererCore.hpp"
#include "renderer/PlatformWindow.hpp"
#include "imgui/imgui.h"
#include "core/Instance.hpp"
#include "core/PhysicalDevice.hpp"
#include "core/LogicalDevice.hpp"
#include "render/Swapchain.hpp"

namespace vpsk {

    RendererCore::RendererCore() {
        ImGui::CreateContext();

        window = std::make_unique<PlatformWindow>(DefaultRendererConfig.Width, DefaultRendererConfig.Height, DefaultRendererConfig.ApplicationName.c_str(), false);
        const VkApplicationInfo application_info{
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            DefaultRendererConfig.ApplicationName.c_str(),
            DefaultRendererConfig.ApplicationVersion,
            DefaultRendererConfig.EngineName.c_str(),
            DefaultRendererConfig.EngineVersion,
            VK_API_VERSION_1_1
        };

        instance = std::make_unique<vpr::Instance>(false, &application_info, window->glfwWindow());
        window->SetWindowUserPointer(this);
        device = std::make_unique<vpr::Device>(instance.get(), instance->GetPhysicalDevice(), false);
        swapchain = std::make_unique<vpr::Swapchain>(instance.get(), device.get());

    }

    std::atomic<bool>& RendererCore::ShouldResize() noexcept {
        std::atomic<bool> flag(false);
        return flag;
    }

    const vpr::Device * RendererCore::Device() const noexcept {
        return device.get();
    }

    vpr::Device * RendererCore::Device() noexcept {
        return device.get();
    }

}

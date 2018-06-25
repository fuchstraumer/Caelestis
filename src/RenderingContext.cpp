#include "RenderingContext.hpp"
#include "PlatformWindow.hpp"
#include "imgui/imgui.h"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "systems/TransferSystem.hpp"
#include "doctest/doctest.h"

namespace vpsk {

    RenderingContext::RenderingContext() {
        ImGui::CreateContext();

        window = std::make_unique<PlatformWindow>(ContextConfiguration.Width, ContextConfiguration.Height, ContextConfiguration.ApplicationName.c_str(), false);
        const VkApplicationInfo application_info{
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            ContextConfiguration.ApplicationName.c_str(),
            ContextConfiguration.ApplicationVersion,
            ContextConfiguration.EngineName.c_str(),
            ContextConfiguration.EngineVersion,
            VK_API_VERSION_1_1
        };

        instance = std::make_unique<vpr::Instance>(vpr::Instance::instance_layers::Full , &application_info, window->glfwWindow());
        window->SetWindowUserPointer(this);
        device = std::make_unique<vpr::Device>(instance.get(), instance->GetPhysicalDevice(), vpr::Device::device_extensions::RecommendedAll);
        swapchain = std::make_unique<vpr::Swapchain>(instance.get(), device.get());
        transferSystem = std::make_unique<ResourceTransferSystem>(device.get());
    }

    RenderingContext::~RenderingContext() {
        transferSystem.reset();
        swapchain.reset();
        window.reset();
        device.reset();
        instance.reset();
    }

    RenderingContext & RenderingContext::GetRenderer() noexcept {
        static RenderingContext renderer;
        return renderer;
    }

    std::atomic<bool>& RenderingContext::ShouldResize() noexcept {
        static std::atomic<bool> flag(false);
        return flag;
    }

    const vpr::Device * RenderingContext::Device() const noexcept {
        return device.get();
    }

    vpr::Device * RenderingContext::Device() noexcept {
        return device.get();
    }

    const vpr::Instance * RenderingContext::Instance() const noexcept {
        return instance.get();
    }

    vpr::Instance * RenderingContext::Instance() noexcept {
        return instance.get();
    }

    const vpr::Swapchain * RenderingContext::Swapchain() const noexcept {
        return swapchain.get();
    }

    vpr::Swapchain * RenderingContext::Swapchain() noexcept {
        return swapchain.get();
    }

    ResourceTransferSystem * RenderingContext::TransferSystem() noexcept {
        return transferSystem.get();
    }

}

#ifdef VPSK_TESTING_ENABLED
TEST_SUITE_BEGIN("RendererCore");

TEST_CASE("RetrieveRenderer") {
    try {
        auto& renderer = vpsk::RenderingContext::GetRenderer();
    }
    catch (...) {
        throw std::runtime_error("Failed to retrieve renderer.");
    }
}

TEST_CASE("RendererRetrieveDevice") {
    vpr::Device* dvc = vpsk::RenderingContext::GetRenderer().Device();
    CHECK(dvc != nullptr);
}

TEST_SUITE_END();
#endif // VPSK_TESTING_ENABLED

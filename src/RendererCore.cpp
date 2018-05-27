#include "RendererCore.hpp"
#include "PlatformWindow.hpp"
#include "imgui/imgui.h"
#include "core/Instance.hpp"
#include "core/PhysicalDevice.hpp"
#include "core/LogicalDevice.hpp"
#include "render/Swapchain.hpp"
#include "systems/TransferSystem.hpp"
#include "doctest/doctest.h"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP
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
        device = std::make_unique<vpr::Device>(instance.get(), instance->GetPhysicalDevice(), true);
        swapchain = std::make_unique<vpr::Swapchain>(instance.get(), device.get());
        transferSystem = std::make_unique<ResourceTransferSystem>(device.get());
    }

    RendererCore::~RendererCore() {
        transferSystem.reset();
        swapchain.reset();
        window.reset();
        device.reset();
        instance.reset();
    }

    RendererCore & RendererCore::GetRenderer() noexcept {
        static RendererCore renderer;
        return renderer;
    }

    std::atomic<bool>& RendererCore::ShouldResize() noexcept {
        static std::atomic<bool> flag(false);
        return flag;
    }

    const vpr::Device * RendererCore::Device() const noexcept {
        return device.get();
    }

    vpr::Device * RendererCore::Device() noexcept {
        return device.get();
    }

    const vpr::Instance * RendererCore::Instance() const noexcept {
        return instance.get();
    }

    vpr::Instance * RendererCore::Instance() noexcept {
        return instance.get();
    }

    const vpr::Swapchain * RendererCore::Swapchain() const noexcept {
        return swapchain.get();
    }

    vpr::Swapchain * RendererCore::Swapchain() noexcept {
        return swapchain.get();
    }

    ResourceTransferSystem * RendererCore::TransferSystem() noexcept {
        return transferSystem.get();
    }

}

#ifdef VPSK_TESTING_ENABLED
TEST_SUITE_BEGIN("RendererCore");

TEST_CASE("RetrieveRenderer") {
    try {
        auto& renderer = vpsk::RendererCore::GetRenderer();
    }
    catch (...) {
        throw std::runtime_error("Failed to retrieve renderer.");
    }
}

TEST_CASE("RendererRetrieveDevice") {
    vpr::Device* dvc = vpsk::RendererCore::GetRenderer().Device();
    CHECK(dvc != nullptr);
}

TEST_SUITE_END();
#endif // VPSK_TESTING_ENABLED

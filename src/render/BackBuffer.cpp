#include "render/BackBuffer.hpp"
#include "core/LogicalDevice.hpp"

namespace vpsk {

    BackBuffer::BackBuffer(const vpr::Device* dvc, const VkSwapchainKHR& swap, const std::vector<std::string>& names) :
        device(dvc), swapchain(swap) {

        for (const auto& name : names) {
            semaphores.emplace(name, VK_NULL_HANDLE);
            VkResult result = vkCreateSemaphore(device->vkHandle(), &vpr::vk_semaphore_create_info_base, nullptr, &semaphores.at(name));
            VkAssert(result);
        }
    }

    BackBuffer::~BackBuffer() {
        for (auto& semaphore : semaphores) {
            if (semaphore.second == VK_NULL_HANDLE) {
                continue;
            }
            vkDestroySemaphore(device->vkHandle(), semaphore.second, nullptr);
        }
    }

    BackBuffer::BackBuffer(BackBuffer&& other) noexcept : device(std::move(other.device)), swapchain(std::move(other.swapchain)), 
        imageIdx(std::move(other.imageIdx)), semaphores(std::move(other.semaphores)) { other.semaphores.clear(); }

    BackBuffer& BackBuffer::operator=(BackBuffer&& other) noexcept {
        device = std::move(other.device);
        swapchain = std::move(other.swapchain);
        imageIdx = std::move(other.imageIdx);
        semaphores = std::move(other.semaphores);
        other.semaphores.clear();
        return *this;
    }

    const VkSemaphore& BackBuffer::GetSemaphore(const std::string& name) const {
        return semaphores.at(name);
    }

    const uint32_t& BackBuffer::GetImageIdx() const noexcept {
        return imageIdx;
    }

}
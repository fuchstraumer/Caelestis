#pragma once
#ifndef VPSK_BACKBUFFER_HPP
#define VPSK_BACKBUFFER_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <map>
namespace vpsk {

    class BackBuffer {
        BackBuffer(const BackBuffer&) = delete;
        BackBuffer& operator=(const BackBuffer&) = delete;
    public:

        BackBuffer(const vpr::Device* dvc, const VkSwapchainKHR& swap, const std::vector<std::string>& semaphores_to_create);
        ~BackBuffer();
        BackBuffer(BackBuffer&& other) noexcept;
        BackBuffer& operator=(BackBuffer&& other) noexcept;

        const VkSemaphore& GetSemaphore(const std::string& name) const;
        const uint32_t& GetImageIdx() const noexcept;

    private:
        VkSwapchainKHR swapchain;
        const vpr::Device* device;
        uint32_t imageIdx = 0;
        std::map<std::string, VkSemaphore> semaphores;
    };

}

#endif //!VPSK_BACKBUFFER_HPP
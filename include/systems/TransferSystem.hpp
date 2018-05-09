#pragma once
#ifndef VPSK_RESOURCE_TRANSFER_SYSTEM_HPP
#define VPSK_RESOURCE_TRANSFER_SYSTEM_HPP
#include "ForwardDecl.hpp"
#include "ecs/signal/Delegate.hpp"
#include <list>
#include <vulkan/vulkan.h>
#include <memory>
#include <unordered_map>

namespace vpsk {

    class VkBufferSystem;

    class ResourceTransferSystem {
    public:

        ResourceTransferSystem(const vpr::Device* dvc);
        void AddTransferRequest(delegate_t<void(VkCommandBuffer)> func);
        void AddTransferRequest(delegate_t<void(VkCommandBuffer)> func, delegate_t<void(VkCommandBuffer)> signal);
        void CompleteTransfers();

    private:
        friend class VkBufferSystem;
        std::unique_ptr<vpr::TransferPool> transferPool;
        // Command buffer to record into, size_t handle to staging, pointer to destination
        std::list<delegate_t<void(VkCommandBuffer)>> transferFunctions;
        std::list<delegate_t<void()>> transferCompleteSignals;
    };

}

#endif //!VPSK_RESOURCE_TRANSFER_SYSTEM_HPP
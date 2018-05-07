#pragma once
#ifndef VPSK_RESOURCE_TRANSFER_SYSTEM_HPP
#define VPSK_RESOURCE_TRANSFER_SYSTEM_HPP
#include "ForwardDecl.hpp"
#include "core/signal/Delegate.hpp"
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
        void CompleteTransfers();

    private:
        friend class VkBufferSystem;
        std::unique_ptr<vpr::TransferPool> transferPool;
        // Command buffer to record into, size_t handle to staging, pointer to destination
        std::list<delegate_t<void(VkCommandBuffer)>> transferFunctions;
    };

}

#endif //!VPSK_RESOURCE_TRANSFER_SYSTEM_HPP
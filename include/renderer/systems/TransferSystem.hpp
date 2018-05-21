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
        ResourceTransferSystem(const ResourceTransferSystem&) = delete;
        ResourceTransferSystem& operator=(const ResourceTransferSystem&) = delete;
    public:

        using TransferDelegate = delegate_t<void(VkCommandBuffer)>;
        using SignalDelegate = delegate_t<void()>;

        ResourceTransferSystem(const vpr::Device* dvc);
        ~ResourceTransferSystem();
        void AddTransferRequest(TransferDelegate func);
        void AddTransferRequest(TransferDelegate func, SignalDelegate signal);
        void CompleteTransfers();

    private:
        friend class VkBufferSystem;
        std::unique_ptr<vpr::TransferPool> transferPool;
        // Command buffer to record into, size_t handle to staging, pointer to destination
        std::list<TransferDelegate> transferFunctions;
        std::list<SignalDelegate> transferCompleteSignals;
    };

}

#endif //!VPSK_RESOURCE_TRANSFER_SYSTEM_HPP
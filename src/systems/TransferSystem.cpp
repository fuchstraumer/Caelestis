#include "systems/TransferSystem.hpp"
#include "command/TransferPool.hpp"
namespace vpsk {

    ResourceTransferSystem::ResourceTransferSystem(const vpr::Device * dvc) : transferPool(std::make_unique<vpr::TransferPool>(dvc)) {
    }

    void ResourceTransferSystem::AddTransferRequest(delegate_t<void(VkCommandBuffer)> func) {
        transferFunctions.emplace_back(std::move(func));
    }

    void ResourceTransferSystem::AddTransferRequest(delegate_t<void(VkCommandBuffer)> func, delegate_t<void(VkCommandBuffer)> signal) {
        transferFunctions.emplace_back(std::move(func));
        transferCompleteSignals.emplace_back(std::move(func));
    }

    void ResourceTransferSystem::CompleteTransfers() {
        auto cmd = transferPool->Begin();
        while (!transferFunctions.empty()) {
            auto& func = transferFunctions.front();
            func(cmd);
            transferFunctions.pop_front();
        }
        transferPool->Submit();

        while (!transferCompleteSignals.empty()) {
            auto& func = transferCompleteSignals.front();
            func();
            transferCompleteSignals.pop_front();
        }
    }

}
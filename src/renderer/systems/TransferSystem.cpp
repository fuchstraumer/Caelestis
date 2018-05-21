#include "renderer/systems/TransferSystem.hpp"
#include "command/TransferPool.hpp"
namespace vpsk {

    ResourceTransferSystem::ResourceTransferSystem(const vpr::Device * dvc) : transferPool(std::make_unique<vpr::TransferPool>(dvc)) {
    }

    ResourceTransferSystem::~ResourceTransferSystem()
    {
    }

    void ResourceTransferSystem::AddTransferRequest(TransferDelegate func) {
        transferFunctions.emplace_back(std::move(func));
    }

    void ResourceTransferSystem::AddTransferRequest(TransferDelegate func, SignalDelegate signal) {
        transferFunctions.emplace_back(std::move(func));
        transferCompleteSignals.emplace_back(std::move(signal));
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
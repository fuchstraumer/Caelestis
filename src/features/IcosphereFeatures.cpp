#include "features/IcosphereFeatures.hpp"

using namespace vpr;

namespace vpsk {

    IcosphereFeatures::IcosphereFeatures(const Device* dvc, const TransferPool* transfer_pool) : device(dvc), transferPool(transfer_pool) {}


    void IcosphereFeatures::AddObject(Icosphere&& ico) {
        objects.push_back(std::move(ico));
    }

    void IcosphereFeatures::setupDescriptorPool() {
        const size_t num_textures_req = objects.size();
        descriptorPool = std::make_unique<DescriptorPool>(device);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(num_textures_req));
        descriptorPool->Create();
    }
}
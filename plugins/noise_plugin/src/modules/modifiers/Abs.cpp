#include "Abs.hpp"
#include "modifiers/abs.cuh"

cnoise::modifiers::Abs::Abs(const size_t& width, const size_t& height, const std::shared_ptr<Module>& previous) : Module(width, height) {
    sourceModules.push_back(previous);
}

size_t cnoise::modifiers::Abs::GetSourceModuleCount() const{
    return 1;
}

void cnoise::modifiers::Abs::Generate() {
    if (sourceModules.front() == nullptr) {
        throw;
    }
    if (!sourceModules.front()->Generated) {
        sourceModules.front()->Generate();
    }
    if (CUDA_LOADED) {     
        cudaAbsLauncher(GetDataPtr(), sourceModules.front()->GetDataPtr(), dims.first, dims.second);
    }
    Generated = true;
}
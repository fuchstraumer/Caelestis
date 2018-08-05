#include "Cache.hpp"
#include "cuda_assert.h"

cnoise::utility::Cache::Cache(const size_t& width, const size_t& height, const std::shared_ptr<Module>& source) : Module(width, height) {
    sourceModules.push_back(source);
}

void cnoise::utility::Cache::Generate(){
    checkSourceModules();

    if (CUDA_LOADED) {
        auto err = cudaDeviceSynchronize();
        cudaAssert(err);

        err = cudaMemcpy(GetDataPtr(), sourceModules.front()->GetDataPtr(), sizeof(float) * sourceModules[0]->Width() * sourceModules[0]->Height(),  cudaMemcpyDefault);
        cudaAssert(err);

        err = cudaDeviceSynchronize();
        cudaAssert(err);
    }
    else {
        // can do simple copy with CPU data.
        const cpu_module_data& cpu_data = std::get<cpu_module_data>(sourceModules.front()->GetVariant());
        this->data = cpu_data;
    }

    // Data copied, remove our reference to the shared pointer
    // this way it'll hopefully be freed
    sourceModules.front().reset();
    sourceModules.clear();
    sourceModules.shrink_to_fit();
}

size_t cnoise::utility::Cache::GetSourceModuleCount() const{
    return 1;
}

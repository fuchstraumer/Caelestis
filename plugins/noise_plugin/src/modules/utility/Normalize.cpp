#include "Normalize.hpp"
#include "utility/normalize.cuh"

cnoise::utility::Normalize::Normalize(const size_t& width, const size_t& height, const std::shared_ptr<Module>& source) : Module(width, height){
    sourceModules.push_back(source);
}

void cnoise::utility::Normalize::Generate(){
    if (sourceModules.front() == nullptr || sourceModules.empty()) {
        throw;
    }
    if (!sourceModules.front()->Generated) {
        sourceModules.front()->Generate();
    }
    //NormalizeLauncher(Output, sourceModules.front()->Output, dims.first, dims.second);
    Generated = true;
}

size_t cnoise::utility::Normalize::GetSourceModuleCount() const{
    return 1;
}

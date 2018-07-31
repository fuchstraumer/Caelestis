#include "ScaleBias.hpp"
#include "modifiers/scalebias.cuh"
#include "modifiers/scalebias.hpp"

cnoise::modifiers::ScaleBias::ScaleBias(const size_t& width, const size_t& height, const float& _scale, const float& _bias) : Module(width, height), scale(_scale), bias(_bias){}

void cnoise::modifiers::ScaleBias::SetBias(const float& _bias){
    bias = _bias;
}

void cnoise::modifiers::ScaleBias::SetScale(const float& _scale){
    scale = _scale;
}

float cnoise::modifiers::ScaleBias::GetBias() const{
    return bias;
}

float cnoise::modifiers::ScaleBias::GetScale() const{
    return scale;
}

size_t cnoise::modifiers::ScaleBias::GetSourceModuleCount() const{
    return 1;
}

void cnoise::modifiers::ScaleBias::Generate(){
    checkSourceModules();
    
    if (CUDA_LOADED) {
        cudaScaleBiasLauncher(GetDataPtr(), sourceModules.front()->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), scale, bias);
    }
    else {
        cpuScaleBiasLauncher(GetDataPtr(), sourceModules[0]->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), scale, bias);
    }

    Generated = true;
}

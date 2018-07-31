#include "DecarpientierSwiss.hpp"
#include "generators/decarpientier_swiss.cuh"
#include "generators/decarpientier_swiss.hpp"

cnoise::generators::DecarpientierSwiss::DecarpientierSwiss(const size_t& width, const size_t& height, const noise_t& noise_type, const float& x, const float& y, const int& seed, const float& freq, const float& lacun, const int& octaves, const float& persist) : Module(width, height), Attributes(seed, freq, lacun, octaves, persist), Origin(x, y), NoiseType(noise_type){}

size_t cnoise::generators::DecarpientierSwiss::GetSourceModuleCount() const{
    return 0;
}

void cnoise::generators::DecarpientierSwiss::Generate(){
    if (CUDA_LOADED) {
        cudaDecarpientierSwissLauncher(GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), NoiseType, make_float2(Origin.first, Origin.second), Attributes.Frequency, Attributes.Lacunarity, Attributes.Persistence, Attributes.Seed, Attributes.Octaves);
    }
    else {
        cpuDecarpientierSwissLauncher(GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), NoiseType, 0.05f, Origin.first, Origin.second, Attributes.Frequency, Attributes.Lacunarity, Attributes.Persistence, Attributes.Octaves, Attributes.Seed);
    }

    Generated = true;
}

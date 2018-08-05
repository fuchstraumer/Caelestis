#include "RidgedMulti.hpp"
#include "generators/ridged_multi.cuh"
#include "generators/ridged_multi.hpp"

namespace cnoise {

    namespace generators {


        RidgedMulti::RidgedMulti(const size_t& width, const size_t& height, const noise_t& noise_type, const float& x, const float& y, const int& seed, const float& freq, const float& lacun, const int& octaves, const float& persist) : Module(width, height),
            Attributes(seed, freq, lacun, octaves, persist), Origin(x, y), NoiseType(noise_type) {}

        size_t RidgedMulti::GetSourceModuleCount() const {
            return 0;
        }

        void RidgedMulti::Generate(){
            if (CUDA_LOADED) {
                cudaRidgedMultiLauncher(GetDataPtr(), dims.first, dims.second, NoiseType, make_float2(Origin.first, Origin.second), Attributes.Frequency, Attributes.Lacunarity, Attributes.Persistence, Attributes.Seed, Attributes.Octaves);
            }
            else {
                cpuRidgedMultiLauncher(GetDataPtr(), dims.first, dims.second, NoiseType, Origin.first, Origin.second, Attributes.Frequency, Attributes.Lacunarity, Attributes.Persistence, Attributes.Octaves, Attributes.Seed);
            }
            Generated = true;
        }

    }

}
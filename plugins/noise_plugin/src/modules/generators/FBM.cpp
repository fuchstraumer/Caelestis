#include "generators/FBM.hpp"

namespace cnoise {
    
    namespace generators {

        // Pass width and height to base class ctor, initialize configuration struct, initialize origin (using initializer list)
        FBM2D::FBM2D(const size_t& width, const size_t& height, const noise_t noise_type, const float& x, const float& y, const int& seed, const float& freq, const float& lacun, const int& octaves, const float& persist) : Module(width, height), 
            Attributes(seed, freq, lacun, octaves, persist), Origin(x,y), NoiseType(noise_type) {}

        // TODO: Implement these. Just here so compiler shuts up.
        size_t FBM2D::GetSourceModuleCount() const{
            return 0;
        }

        void FBM2D::Generate(){

            Generated = true;
        }
    }
}

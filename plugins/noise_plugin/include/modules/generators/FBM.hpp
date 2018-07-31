#pragma once
#ifndef FBM_H
#define FBM_H
#include "Base.hpp"

/*

    Generates values using FBM noise. Ctor includes desired dimensions
    of output texture.
    
    These classes must exist as wrappers over the CUDA kernels since we need
    to allocate and create our constant texture objects, used as permutation
    tables and the like. Without these, we have to use costly/slow array lookups.

*/
namespace cnoise {

    namespace generators {

        // Default parameters
        constexpr float DEFAULT_FBM_FREQUENCY = 1.0f;
        constexpr float DEFAULT_FBM_LACUNARITY = 2.0f;
        constexpr int DEFAULT_FBM_OCTAVES = 6;
        constexpr float DEFAULT_FBM_PERSISTENCE = 0.50f;
        constexpr int DEFAULT_FBM_SEED = 0;

        // Maximum octave level to allow
        constexpr int FBM_MAX_OCTAVES = 24;


        class FBM2D : public Module {
        public:

            // Width + height specify output texture size.
            // Seed defines a value to seed the generator with
            // X & Y define the origin of the noise generator
            FBM2D(const size_t& width, const size_t& height, const noise_t noise_type, const float& x, const float& y, const int& seed, const float& freq, const float& lacun, const int& octaves, const float& persist);

            // Get source module count: must be 0, this is a generator and can't have preceding modules.
            virtual size_t GetSourceModuleCount() const override;

            // Launches the kernel and fills this object's surface object with the relevant data.
            virtual void Generate() override;

            // Origin of this noise generator. Keep the seed constant and change this for 
            // continuous "tileable" noise
            std::pair<float, float> Origin;

            // Configuration attributes.
            noiseCfg Attributes;

            // Type of noise to use.
            noise_t NoiseType;
        };

        class FBM3D {
        public:

        };

        class FBM4D {
        public:

        };
    }
}

#endif // !FBM_H

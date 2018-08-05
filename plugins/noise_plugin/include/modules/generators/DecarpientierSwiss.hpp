#pragma once
#ifndef DECARPIENTIER_SWISS_H
#define DECARPIENTIER_SWISS_H
#include "Base.hpp"

namespace cnoise {

    namespace generators {

        // Default parameters
        constexpr float DEFAULT_DC_SWISS_FREQUENCY = 0.25f;
        constexpr float DEFAULT_DC_SWISS_LACUNARITY = 2.0f;
        constexpr int DEFAULT_DC_SWISS_OCTAVES = 10;
        constexpr float DEFAULT_DC_SWISS_PERSISTENCE = 0.50f;
        constexpr int DEFAULT_DC_SWISS_SEED = 0;

        // Maximum octave level to allow
        constexpr int DC_SWISS_MAX_OCTAVES = 24;

        class DecarpientierSwiss : public Module {
        public:
            // Width + height specify output texture size.
            // Seed defines a value to seed the generator with
            // X & Y define the origin of the noise generator
            DecarpientierSwiss(const size_t& width, const size_t& height, const noise_t& noise_type, const float& x, const float& y, const int& seed, const float& freq, const float& lacun, const int& octaves, const float& persist);

            // Get source module count: must be 0, this is a generator and can't have preceding modules.
            virtual size_t GetSourceModuleCount() const override;

            // Launches the kernel and fills this object's surface object with the relevant data.
            virtual void Generate() override;

            // Origin of this noise generator. Keep the seed constant and change this for 
            // continuous "tileable" noise
            std::pair<float, float> Origin;

            // Configuration attributes.
            noiseCfg Attributes;

            noise_t NoiseType;
        };

    }

}
#endif // !DECARPIENTIER_SWISS_H

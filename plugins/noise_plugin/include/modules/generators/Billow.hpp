#pragma once
#ifndef BILLOW_H
#define BILLOW_H
#include "Base.hpp"

namespace cnoise {

        namespace generators {

            // Default parameters
            constexpr float DEFAULT_BILLOW_FREQUENCY = 0.25f;
            constexpr float DEFAULT_BILLOW_LACUNARITY = 2.0f;
            constexpr int DEFAULT_BILLOW_OCTAVES = 6;
            constexpr float DEFAULT_BILLOW_PERSISTENCE = 0.50f;
            constexpr int DEFAULT_BILLOW_SEED = 0;

            // Maximum octave level to allow
            constexpr int BILLOW_MAX_OCTAVES = 24;


            class Billow2D : public Module {
            public:

                // Width + height specify output texture size.
                // Seed defines a value to seed the generator with
                // X & Y define the origin of the noise generator
                Billow2D(const size_t& width, const size_t& height, const noise_t& noise_type, const float& x, const float& y, const int& seed, const float& freq, const float& lacun, const int& octaves, const float& persist);

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

            /*
            class Billow3D : public Module3D {
            public:

                // Width + height specify output texture size.
                // Seed defines a value to seed the generator with
                // X & Y define the origin of the noise generator
                Billow3D(int width, int height, int depth = 1, float x = 0.0f, float y = 0.0f, float z = 0.0f, int seed = DEFAULT_BILLOW_SEED, float freq = DEFAULT_BILLOW_FREQUENCY, float lacun = DEFAULT_BILLOW_LACUNARITY,
                    int octaves = DEFAULT_BILLOW_OCTAVES, float persist = DEFAULT_BILLOW_PERSISTENCE);

                // Get source module count: must be 0, this is a generator and can't have preceding modules.
                virtual size_t GetSourceModuleCount() const override;

                // Launches the kernel and fills this object's surface object with the relevant data.
                virtual void Generate() override;

                // Origin of this noise generator. Keep the seed constant and change this for 
                // continuous "tileable" noise
                float3 Origin;

                // Configuration attributes.
                noiseCfg Attributes;

                noise_t NoiseType;

            };
            */
        }

}


#endif // !BILLOW_H

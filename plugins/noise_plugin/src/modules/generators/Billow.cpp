#include "generators/Billow.hpp"

namespace cnoise {

        namespace generators {

            Billow2D::Billow2D(const size_t& width, const size_t& height, const noise_t& noise_type, const float& x, const float& y, const int& seed, const float& freq, const float& lacun, const int& octaves, const float& persist) : Module(width, height), 
                Attributes(seed, freq, lacun, octaves, persist), Origin(x, y), NoiseType(noise_type) {}

            size_t Billow2D::GetSourceModuleCount() const {
                return 0;
            }

            void Billow2D::Generate() {

                Generated = true;
            }

        }
}

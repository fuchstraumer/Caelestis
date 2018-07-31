#pragma once
#ifndef TURBULENCE_H
#define TURBULENCE_H
#include "Base.hpp"

namespace cnoise {

    namespace modifiers {

        constexpr int DEFAULT_TURBULENCE_ROUGHNESS = 3;
        constexpr int DEFAULT_TURBULENCE_SEED = 0;
        constexpr float DEFAULT_TURBULENCE_STRENGTH = 3.0f;
        constexpr float DEFAULT_TURBULENCE_FREQUENCY = 0.05f;

        class Turbulence : public Module {
        public:

            Turbulence(const size_t& width, const size_t& height, const std::shared_ptr<Module>& prev, const int& roughness, const int& seed, const float& strength, const float& freq);

            virtual size_t GetSourceModuleCount() const override;

            virtual void Generate() override;

            void SetStrength(const float& _strength);
            float GetStrength() const;
            void SetSeed(int _seed);
            int GetSeed() const;
            void SetRoughness(const int& _rough);
            int GetRoughness() const;
            float GetFrequency() const;
            void SetFrequency(const float& _freq);

        private:

            float strength;
            float frequency;
            int seed;
            int roughness;
            noise_t noiseType;

        };
    
    }

}

#endif // !TURBULENCE_H

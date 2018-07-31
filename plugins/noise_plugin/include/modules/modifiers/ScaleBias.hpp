#pragma once
#ifndef SCALE_BIAS_H
#define SCALE_BIAS_H
#include "Base.hpp"

namespace cnoise {

    namespace modifiers {

        class ScaleBias : public Module {
        public:

            ScaleBias(const size_t& width, const size_t& height, const float& _scale, const float& _bias);

            void SetBias(const float& _bias);
            
            void SetScale(const float& _scale);

            float GetBias() const;

            float GetScale() const;

            virtual size_t GetSourceModuleCount() const override;

            void Generate() override;

        private:

            float bias, scale;

        };

    }

}
#endif // !SCALE_BIAS_H

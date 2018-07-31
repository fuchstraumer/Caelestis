#pragma once
#ifndef CLAMP_H
#define CLAMP_H
#include "Base.hpp"

namespace cnoise {

    namespace modifiers {

        class Clamp : public Module {
        public:

            Clamp(const size_t& width, const size_t& height, const float& lower_bound, const float& upper_bound, const std::shared_ptr<Module>& source);

            virtual size_t GetSourceModuleCount() const override;

            virtual void Generate() override;

            float GetLowerBound() const;

            float GetUpperBound() const;

            void SetLowerBound(const float& lower);

            void SetUpperBound(const float& upper);

        private:

            float lowerBound, upperBound;

        };

    }

}
#endif // !CLAMP_H

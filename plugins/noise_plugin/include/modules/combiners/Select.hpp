#pragma once
#ifndef SELECT_H
#define SELECT_H
#include "Base.hpp"

namespace cnoise {

    namespace combiners {

        /*

            Select module:

            A selector/select module chooses which of two 
            connected subject modules to sample a value from 
            using a selector module.

            For this other module, a range of values is set 
            (along with an optional falloff value), and if 
            the value falls within this range subject0 is chosen.
            If it is outside of the range, subject1 is chosen.

            The falloff value eases the transition between
            the two module's values in the final output.
        
        */


        class Select : public Module {
        public:

            // The falloff value does not have to be set at all. The Modules MUST be set eventually, but don't need to be set upon initialization.
            Select(const size_t& width, const size_t& height, const float& low_value, const float& high_value, const float& falloff, const std::shared_ptr<Module>& selector, const std::shared_ptr<Module>& subject0, const std::shared_ptr<Module>& subject1);

            // Set subject module
            void SetSubject(const size_t& idx, const std::shared_ptr<Module>& subject);

            // Set selector module
            void SetSelector(const std::shared_ptr<Module>& selector);

            // SourceModule count = 3
            virtual size_t GetSourceModuleCount() const override;

            // Run kernel.
            virtual void Generate() override;

            // Set high/low values
            void SetHighThreshold(float _high);
            void SetLowThreshold(float _low);

            // Get high/low values
            float GetHighTreshold() const;
            float GetLowThreshold() const;

            // Set falloff
            void SetFalloff(float _falloff);

            // Get falloff
            float GetFalloff() const;

        private:

            float highThreshold, lowThreshold;

            float falloff;

        };

    }
}
#endif // !SELECT_H

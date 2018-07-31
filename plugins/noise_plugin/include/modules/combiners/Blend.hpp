#pragma once
#ifndef BLEND_H
#define BLEND_H
#include "Base.hpp"

namespace cnoise {

    namespace combiners {

        class Blend : public Module {
        public:

            Blend(const size_t& width, const size_t& height, const std::shared_ptr<Module>& in0, const std::shared_ptr<Module>& in1, const std::shared_ptr<Module>& weight_module);

            virtual void Generate() override;

            virtual size_t GetSourceModuleCount() const override;

            void SetSourceModule(const size_t& idx, const std::shared_ptr<Module>& source);

            void SetControlModule(const std::shared_ptr<Module>& control);

        };

    }

}
#endif // !BLEND_H

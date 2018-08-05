#pragma once
#ifndef POWER_H
#define POWER_H
#include "Base.hpp"

namespace cnoise {

    namespace combiners {

        class Power : public Module {
        public:

            Power(const size_t& width, const size_t& height, const std::shared_ptr<Module>& in0, const std::shared_ptr<Module>& in1);

            virtual void Generate() override;

            virtual size_t GetSourceModuleCount() const override;

        };

    }

}

#endif // !POWER_H

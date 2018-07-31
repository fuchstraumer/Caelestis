#pragma once
#ifndef CONSTANT_H
#define CONSTANT_H
#include "Base.hpp"

namespace cnoise {

    namespace utility {

        class Constant : public Module {
        public:
            Constant(const size_t& width, const size_t& height, const float& value);

            virtual size_t GetSourceModuleCount() const override;

            virtual void Generate() override;
        };
    }

}

#endif // !CONSTANT_H

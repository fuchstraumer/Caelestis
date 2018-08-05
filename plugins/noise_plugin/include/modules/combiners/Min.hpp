#pragma once
#ifndef MIN_H
#define MIN_H
#include "Base.hpp"

namespace cnoise {

    namespace combiners {

        class Min : public Module {
        public:

            Min(const size_t& width, const size_t& height, const std::shared_ptr<Module>& in0, const std::shared_ptr<Module>& in1);

            virtual void Generate() override;

            virtual size_t GetSourceModuleCount() const override;

        };

    }

}

#endif // !MIN_H

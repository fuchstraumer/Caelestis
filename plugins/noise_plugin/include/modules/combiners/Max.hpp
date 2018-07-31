#pragma once
#ifndef MAX_H
#define MAX_H
#include "Base.hpp"

namespace cnoise {

    namespace combiners {

        class Max : public Module {
        public:

            Max(const size_t& width, const size_t& height, const std::shared_ptr<Module>& in0, const std::shared_ptr<Module>& in1);

            virtual void Generate() override;

            virtual size_t GetSourceModuleCount() const override;

        };

    }

}
#endif // !MAX_H

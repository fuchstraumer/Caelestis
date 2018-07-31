#pragma once
#ifndef NORMALIZE_H
#define NORMALIZE_H
#include "Base.hpp"

namespace cnoise {

    namespace utility {

        class Normalize : public Module {
        public:

            Normalize(const size_t& width, const size_t& height, const std::shared_ptr<Module>& source);

            virtual void Generate() override;

            virtual size_t GetSourceModuleCount() const override;

        };

    }

}

#endif // !NORMALIZE_H

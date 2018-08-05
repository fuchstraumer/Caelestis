#pragma once
#ifndef CACHE_H
#define CACHE_H
#include "Base.hpp"

namespace cnoise {
    
    namespace utility {

        /*
        
            Module - Cache.

            "Caches" values by running connected source module, which
            recurses to its connected source modules, and so on. 

            More useful with CPU-side noise libraries than this one,
            but helps to "stage" our calculations if we want to perform
            sanity checks in between them.
        
        
        */

        class Cache : public Module {
        public:

            Cache(const size_t& width, const size_t& height, const std::shared_ptr<Module>& source);

            virtual void Generate() override;

            virtual size_t GetSourceModuleCount() const override;

        };

    }

}

#endif // !CACHE_H

#include "combiners/Add.hpp"
#include <iostream>

namespace cnoise {

    namespace combiners {

        Add::Add(int width, int height, float add_value, Module* source) : Module(width, height) {
            sourceModules.front() = std::shared_ptr<Module>(source);
        }

        void Add::Generate(){
            checkSourceModules();

            Generated = true;
        }

        size_t Add::GetSourceModuleCount() const{
            return 2;
        }

    }
}


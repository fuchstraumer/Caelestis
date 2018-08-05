#include "Add.hpp"
#include <iostream>

#include "combiners/add.cuh"
#include "combiners/add.hpp"

namespace cnoise {

    namespace combiners {

        Add::Add(int width, int height, float add_value, Module* source) : Module(width, height) {
            sourceModules.front() = std::shared_ptr<Module>(source);
        }

        void Add::Generate(){
            checkSourceModules();
            
            if (CUDA_LOADED) {
                cudaAddLauncher(GetDataPtr(), sourceModules[0]->GetDataPtr(), sourceModules[1]->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second));
            }
            else {
                cpuAddLauncher(GetDataPtr(), sourceModules.front()->GetDataPtr(), sourceModules[1]->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second));
            }

            Generated = true;
        }

        size_t Add::GetSourceModuleCount() const{
            return 2;
        }

    }
}


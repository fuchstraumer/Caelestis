#include "Select.hpp"
#include <iostream>
#include "combiners/select.cuh"
#include "combiners/select.hpp"

namespace cnoise {

    namespace combiners {

        Select::Select(const size_t& width, const size_t& height, const float& low_value, const float& high_value, const float& _falloff, const std::shared_ptr<Module>& selector, const std::shared_ptr<Module>& subject0, const std::shared_ptr<Module>& subject1) : Module(width, height), 
            lowThreshold(low_value), highThreshold(high_value), falloff(_falloff)  {
            sourceModules.resize(3);
            sourceModules[0] = selector;
            sourceModules[1] = subject0;
            sourceModules[2] = subject1;
        }

        void Select::SetSubject(const size_t& idx, const std::shared_ptr<Module>& subject){
            if (idx > 2 || idx < 1) {
                std::cerr << "Index supplied to SetSubject method of a Select module must be 1 or 2 - First subject, or second subject." << std::endl;
                throw("Invalid index supplied");
            }
            else {
                sourceModules[idx] = subject;
            }
        }

        void Select::SetSelector(const std::shared_ptr<Module>& selector){
            sourceModules[0] = selector;
        }

        void Select::Generate() {
            checkSourceModules();

            if (CUDA_LOADED) {
                cudaSelectLauncher(GetDataPtr(), sourceModules[0]->GetDataPtr(), sourceModules[1]->GetDataPtr(), sourceModules[2]->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), highThreshold, lowThreshold, falloff);
            }
            else {
                cpuSelectLauncher(GetDataPtr(), sourceModules[0]->GetDataPtr(), sourceModules[1]->GetDataPtr(), sourceModules[2]->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), highThreshold, lowThreshold, falloff);
            }

            Generated = true;
        }

        void Select::SetHighThreshold(float _high){
            highThreshold = _high;
        }

        void Select::SetLowThreshold(float _low){
            lowThreshold = _low;
        }

        float Select::GetHighTreshold() const{
            return highThreshold;
        }

        float Select::GetLowThreshold() const{
            return lowThreshold;
        }

        void Select::SetFalloff(float _falloff){
            falloff = _falloff;
        }

        float Select::GetFalloff() const{
            return falloff;
        }

        size_t Select::GetSourceModuleCount() const {
            return 3;
        }

    }

}

#include "combiners/Blend.hpp"

cnoise::combiners::Blend::Blend(const size_t& width, const size_t& height, const std::shared_ptr<Module>& in0 = nullptr, const std::shared_ptr<Module>& in1 = nullptr, const std::shared_ptr<Module>& weight_module = nullptr) : Module(width, height) {
    sourceModules.push_back(in0);
    sourceModules.push_back(in1);
    sourceModules.push_back(weight_module);
}

void cnoise::combiners::Blend::Generate() {

    Generated = true;
}

size_t cnoise::combiners::Blend::GetSourceModuleCount() const{
    return 3;
}

void cnoise::combiners::Blend::SetSourceModule(const size_t& idx, const std::shared_ptr<Module>& source){
    if (idx > 2 || idx < 0) {
        return;
    }
    sourceModules[idx] = source;
}

void cnoise::combiners::Blend::SetControlModule(const std::shared_ptr<Module>& control){
    sourceModules[2] = control;
}

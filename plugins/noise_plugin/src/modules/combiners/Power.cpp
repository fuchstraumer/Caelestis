#include "combiners/Power.hpp"

cnoise::combiners::Power::Power(const size_t& width, const size_t& height, const std::shared_ptr<Module>& in0, const std::shared_ptr<Module>& in1) : Module(width, height){
    sourceModules.push_back(in0);
    sourceModules.push_back(in1);
}

void cnoise::combiners::Power::Generate(){
    checkSourceModules();

    Generated = true;
}

size_t cnoise::combiners::Power::GetSourceModuleCount() const{
    return 2;
}

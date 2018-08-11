#include "combiners/Max.hpp"

cnoise::combiners::Max::Max(const size_t& width, const size_t& height, const std::shared_ptr<Module>& in0 = nullptr, const std::shared_ptr<Module>& in1 = nullptr) : Module(width, height) {
    sourceModules.push_back(in0);
    sourceModules.push_back(in1);
}

void cnoise::combiners::Max::Generate() {
    checkSourceModules();


    Generated = true;
}

size_t cnoise::combiners::Max::GetSourceModuleCount() const{
    return 2;
}

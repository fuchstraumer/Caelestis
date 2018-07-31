#include "Checkerboard.hpp"
#include "utility/checkerboard.cuh"

cnoise::utility::Checkerboard::Checkerboard(const size_t& width, const size_t& height) : Module(width, height) {}

void cnoise::utility::Checkerboard::Generate(){
    //CheckerboardLauncher(Output, dims.first, dims.second);
    Generated = true;
}

size_t cnoise::utility::Checkerboard::GetSourceModuleCount() const{
    return 0;
}

#include "Constant.hpp"
#include "cuda_assert.h"

cnoise::utility::Constant::Constant(const size_t& width, const size_t& height, const float& value) : Module(width, height) {
    cudaError_t err = cudaDeviceSynchronize();
    cudaAssert(err);

    std::vector<float> constant_val;
    constant_val.assign(width * height, value);

    //err = cudaMemcpy(Output, &constant_val[0], width * height * sizeof(float), cudaMemcpyHostToDevice);
    //cudaAssert(err);

    err = cudaDeviceSynchronize();
    cudaAssert(err);

    Generated = true;
}

size_t cnoise::utility::Constant::GetSourceModuleCount() const{
    return 0;
}

void cnoise::utility::Constant::Generate(){}

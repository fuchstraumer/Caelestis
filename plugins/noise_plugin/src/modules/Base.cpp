#include "Base.hpp"
#include "cuda_assert.h"
#include "../image/Image.hpp"
#include "utility/normalize.cuh"
#include <algorithm>

namespace cnoise {

    bool Module::CUDA_LOADED = false;
    bool Module::VULKAN_LOADED = false;
    
    Module::Module(const size_t& width, const size_t& height) : dims(width, height) {
            Generated = false;
            // Allocate using managed memory, so that CPU/GPU can share a single pointer.
            // Be sure to call cudaDeviceSynchronize() before accessing Output.
            if (CUDA_LOADED) {
                data = cuda_module_data(width, height);
            }
            else {
                data = cpu_module_data(width, height);
            }
    }

    void Module::ConnectModule(const std::shared_ptr<Module>& other) {
        if (sourceModules.size() < GetSourceModuleCount()) {
            sourceModules.push_back(other);
        }
    }

    std::vector<float> Module::GetData() const{
        if (CUDA_LOADED) {
            auto& cu_struct = std::get<cuda_module_data>(data);
            return cu_struct.GetData();
        }
        else {
            auto& cpu_struct = std::get<cpu_module_data>(data);
            return cpu_struct.data;
        }
    }

    Module& Module::GetModule(size_t idx) const {
        // .at(idx) has bounds checking in debug modes, iirc.
        return *sourceModules.at(idx);
    }

    std::vector<float> Module::GetDataNormalized(float upper_bound, float lower_bound) const {
        if (CUDA_LOADED) {
            cudaError_t err = cudaDeviceSynchronize();
            err = cudaDeviceSynchronize();
            cudaAssert(err);
            float* norm;
            err = cudaMallocManaged(&norm, dims.first * dims.second * sizeof(float));
            cudaAssert(err);

            auto& cu_struct = std::get<cuda_module_data>(data);

            //cudaNormalizeLauncher(norm, cu_struct.data, static_cast<int>(dims.first), static_cast<int>(dims.second));

            err = cudaDeviceSynchronize();
            cudaAssert(err);

            auto result = std::vector<float>(norm, norm + (dims.first * dims.second));
            cudaFree(norm);
            return std::move(result);
        }
        else {
            auto& var = std::get<cpu_module_data>(data);
            auto min_max = std::minmax_element(var.data.cbegin(), var.data.cend());
            const float& min = *min_max.first;
            const float& max = *min_max.second;
            std::vector<float> result(var.data.cbegin(), var.data.cend());
            for(auto& elem : result) {
                elem = (elem - min) / (max - min);
            }
            return result;
        }
    }
    
    void Module::SaveToPNG(const char * name){
        std::vector<float> rawData = GetData();
        img::ImageWriter out(static_cast<int>(dims.first), static_cast<int>(dims.second));
        out.SetRawData(rawData);
        out.WritePNG(name);
    }

    void Module::SaveToPNG_16(const char * filename) {
        std::vector<float> raw = GetData();
        img::ImageWriter out(static_cast<int>(dims.first), static_cast<int>(dims.second));
        out.SetRawData(raw);
        out.WritePNG_16(filename);
    }

    void Module::SaveRaw32(const char* filename) {
        std::vector<float> raw = GetData();
        img::ImageWriter out(static_cast<int>(dims.first), static_cast<int>(dims.second));
        out.SetRawData(raw);
        out.WriteRaw32(filename);
    }

    void Module::SaveToTER(const char * name) {
        std::vector<float> rawData = GetData();
        img::ImageWriter out(static_cast<int>(dims.first), static_cast<int>(dims.second));
        out.SetRawData(rawData);
        out.WriteTER(name);
    }

    float* Module::GetDataPtr() {
        if (CUDA_LOADED) {
            auto& cu_data = std::get<cuda_module_data>(data);
            return cu_data.data;
        }
        else {
            auto& cpu_data = std::get<cpu_module_data>(data);
            return cpu_data.data.data();
        }
    }

    const size_t & Module::Width() const noexcept {
        return dims.first;
    }

    const size_t & Module::Height() const noexcept {
        return dims.second;
    }

    void Module::checkSourceModules() {
        for (const auto m : sourceModules) {
            if (m == nullptr) {
                throw std::runtime_error("Source module in sourceModules was nullptr!");
            }
            if (!m->Generated) {
                m->Generate();
            }
        }
    }

    cuda_module_data::cuda_module_data(const size_t & w, const size_t & h) : width(w), height(h) {
        auto err = cudaMallocManaged(&data, sizeof(float) * width * height);
        cudaAssert(err);
        err = cudaDeviceSynchronize();
        cudaAssert(err);
    }

    cuda_module_data::cuda_module_data(cuda_module_data && other) noexcept {
        data = std::move(other.data);
        width = std::move(other.width);
        height = std::move(other.height);
        other.data = nullptr;
    }

    cuda_module_data & cuda_module_data::operator=(cuda_module_data && other) noexcept {
        data = std::move(other.data);
        other.data = nullptr;
        width = std::move(other.width);
        height = std::move(other.height);
        return *this;
    }

    cuda_module_data::~cuda_module_data() {
        if (data != nullptr) {
            auto err = cudaDeviceSynchronize();
            cudaAssert(err);
            err = cudaFree(data);
            cudaAssert(err);
        }
    }

    std::vector<float> cuda_module_data::GetData() const noexcept {
        // Make sure to sync device before trying to get data.
        auto err = cudaDeviceSynchronize();
        cudaAssert(err);
        std::vector<float> result(data, data + (width * height));
        return result;
    }

    cpu_module_data::cpu_module_data(const size_t & w, const size_t & h) : width(w), height(h) {
        data.resize(w * h);
    }

}


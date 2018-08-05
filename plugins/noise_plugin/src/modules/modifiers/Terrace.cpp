#include "Terrace.hpp"
#include "modifiers/terrace.cuh"
#include "modifiers/terrace.hpp"
#include <cassert>

cnoise::modifiers::Terrace::Terrace(const size_t& width, const size_t& height) : Module(width, height), inverted(false) {}

void cnoise::modifiers::Terrace::Generate(){
    checkSourceModules();
    std::vector<float> points = std::vector<float>(controlPoints.cbegin(), controlPoints.cend());
    assert(!points.empty());

    if (CUDA_LOADED) {
        cudaTerraceLauncher(GetDataPtr(), sourceModules.front()->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), points.data(), static_cast<int>(points.size()), inverted);
    }
    else {
        cpuTerraceLauncher(GetDataPtr(), sourceModules[0]->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), points.data(), static_cast<int>(points.size()), inverted);
    }

    Generated = true;
}

size_t cnoise::modifiers::Terrace::GetSourceModuleCount() const{
    return 1;
}

void cnoise::modifiers::Terrace::AddControlPoint(const float & val){
    controlPoints.insert(val);
}

void cnoise::modifiers::Terrace::ClearControlPoints(){
    controlPoints.clear();
}

void cnoise::modifiers::Terrace::MakeControlPoints(const size_t & num_pts){
    ClearControlPoints();
    float step = 1.0f / num_pts;
    for (size_t i = 0; i < num_pts; ++i) {
        controlPoints.insert(step * i);
    }
}

void cnoise::modifiers::Terrace::SetInversion(bool inv){
    inverted = inv;
}

bool cnoise::modifiers::Terrace::GetInversion() const{
    return inverted;
}

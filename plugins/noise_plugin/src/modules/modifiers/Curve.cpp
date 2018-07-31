#include "Curve.hpp"
#include "modifiers/curve.cuh"
#include "modifiers/curve.hpp"
#include <iostream>
namespace cnoise {

    namespace modifiers {

        Curve::Curve(const size_t& width, const size_t& height) : Module(width, height) {}

        Curve::Curve(const size_t& width, const size_t& height, const std::vector<ControlPoint>& init_points) : Module(width, height), controlPoints(init_points) {}

        void Curve::AddControlPoint(const float& input_val, const float& output_val){
            controlPoints.push_back(ControlPoint(input_val, output_val));
        }

        size_t Curve::GetSourceModuleCount() const {
            return 1;
        }

        const std::vector<ControlPoint>& Curve::GetControlPoints() const{
            return controlPoints;
        }

        void Curve::SetControlPoints(const std::vector<ControlPoint>& pts){
            controlPoints.clear();
            controlPoints = pts;
        }

        void Curve::ClearControlPoints(){
            controlPoints.clear();
            controlPoints.shrink_to_fit();
        }

        void Curve::Generate(){
            checkSourceModules();
            if (CUDA_LOADED) {
                cudaCurveLauncher(GetDataPtr(), sourceModules.front()->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), controlPoints.data(), static_cast<int>(controlPoints.size()));
            }
            else {
                cpuCurveLauncher(GetDataPtr(), sourceModules[0]->GetDataPtr(), static_cast<int>(dims.first), static_cast<int>(dims.second), controlPoints.data(), static_cast<int>(controlPoints.size()));
            }
            Generated = true;
        }
    }
}
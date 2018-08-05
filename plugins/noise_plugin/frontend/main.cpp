// Include modules
#define USING_CNOISE_NAMESPACES
#include "modules/Modules.hpp"
#include "modules/utility/Cache.hpp"
#include "image/Image.hpp"
#include "cuda_assert.h"
#include <cuda_runtime.h>
#include <iostream>

// Check for CUDA support
int cuda_supported() {
    int result = 0;
    auto err = cudaGetDeviceCount(&result);
    return result;
}

int main() {
    int cuda_check = cuda_supported();
    if (cuda_check != 0) {
        cnoise::Module::CUDA_LOADED = true;
    }
    else {
        cnoise::Module::CUDA_LOADED = false;
    }

    using namespace cnoise;
    using namespace cnoise::utility;
    using namespace cnoise::modifiers;

    constexpr size_t img_size_x = 8192;
    constexpr size_t img_size_y = 8192;
    constexpr float sea_level = 0.10f;
    constexpr float continent_freq = 0.0008f;
    constexpr float continent_lacun = 2.30f;

    // Base continent module.
    std::shared_ptr<Billow2D> baseContinentDefinition_pe0 = std::make_shared<Billow2D>(img_size_x, img_size_y, noise_t::SIMPLEX, 0.0f, 0.0f, 123132, continent_freq, continent_lacun, 13, 0.95f);

    // Curve output so that very high values are near sea level, which defines the positions of the mountain ranges.
    std::shared_ptr<Curve> baseContinentDefinition_cu0 = std::make_shared<Curve>(img_size_x, img_size_y);
    baseContinentDefinition_cu0->ConnectModule(baseContinentDefinition_pe0);
    std::vector<ControlPoint> base_cu0_pts = {
        ControlPoint(-2.00f + sea_level, -1.625f + sea_level),
        ControlPoint(-1.00f + sea_level, -1.375f + sea_level),
        ControlPoint(0.000f + sea_level, -0.375f + sea_level),
        ControlPoint(0.0625f + sea_level, 0.125f + sea_level),
        ControlPoint(0.1250f + sea_level, 0.250f + sea_level),
        ControlPoint(0.2500f + sea_level, 1.000f + sea_level),
        ControlPoint(0.5000f + sea_level, 0.250f + sea_level),
        ControlPoint(0.7500f + sea_level, 0.250f + sea_level),
        ControlPoint(1.0000f + sea_level, 0.500f + sea_level),
        ControlPoint(2.0000f + sea_level, 0.500f + sea_level),
    };
    baseContinentDefinition_cu0->SetControlPoints(base_cu0_pts);

    std::shared_ptr<Billow2D> baseContinentDefinition_pe1 = std::make_shared<Billow2D>(img_size_x, img_size_y, noise_t::SIMPLEX, 0.0f, 0.0f, 1213213, continent_freq * 4.0f, continent_lacun, 15, 0.75f);
    std::shared_ptr<ScaleBias> baseContinentDef_sb = std::make_shared<ScaleBias>(img_size_x, img_size_y, 0.375f, 0.625f);
    baseContinentDef_sb->ConnectModule(baseContinentDefinition_pe1);
    std::shared_ptr<Min> baseContinentDef_min0 = std::make_shared<Min>(img_size_x, img_size_y, baseContinentDef_sb, baseContinentDefinition_cu0);
    std::shared_ptr<Cache> baseContinentDef_ca1 = std::make_shared<Cache>(img_size_x, img_size_y, baseContinentDef_min0);
    baseContinentDef_ca1->ConnectModule(baseContinentDef_min0);
    baseContinentDef_ca1->Generate();

    //// This module carves out chunks from the curve module used to set ranges for continents, by selecting the min
    //// between the carver chain and the base continent chain.
    //Min3D baseContinentDef_min0(img_size_x, img_size_y, &baseContinentDef_sb, &baseContinentDef_ca0);


    //Cache3D baseContinentDef_ca1(img_size_x, img_size_y, &baseContinentDef_min0);

    //Turbulence3D baseContinentDef_tu0(img_size_x, img_size_y, &baseContinentDef_ca1, 13, 1341324, 10.0f, continent_freq);
    //baseContinentDef_tu0.Generate();
    //////cudaDeviceSynchronize();
    //baseContinentDef_tu0.SaveToPNG("turbulence.png");
    
    cnoise::img::ImageWriter img(img_size_x, img_size_y);
    img.SetRawData(baseContinentDefinition_cu0->GetData());
    img.WriteRaw32("data0.img");
    img.SetRawData(baseContinentDef_ca1->GetData());
    img.WriteRaw32("data1.img");


}
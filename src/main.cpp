#include <string>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <random>
#include <iostream>
#include "renderer/RendererCore.hpp"
#include "renderer/resources/stbTexture.hpp"
#include "renderer/resources/gliTexture.hpp"

#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP


int main(int argc, char* argv[]) {
    using namespace vpsk;
    auto& renderer = RendererCore::GetRenderer();
    stbTexture texture1(renderer.Device(), "SciFiHelmet_BaseColor.png");
    //gliTexture<gli::texture2d> texture0(renderer.Device(), "albedo.ktx");
}
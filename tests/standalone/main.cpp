#include "RenderingContext.hpp"
#include "graph/RenderGraph.hpp"
#include "systems/BufferResourceCache.hpp"
#include "systems/ImageResourceCache.hpp"
#include "core/ShaderResource.hpp"
#include "core/ShaderPack.hpp"
#include "generation/ShaderGenerator.hpp"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
    std::vector<std::string> args{ argv + 1, argv + argc };
    using namespace st;
    using namespace vpsk;

    const std::string working_dir_path("../tests/ClusteredForward/");

    ShaderGenerator::SetBasePath("../third_party/shadertools/fragments/");
    ShaderPack pack(std::string(working_dir_path + "shaders/Pack.lua").c_str());

    RenderingContext& renderer = RenderingContext::GetRenderer();
    RenderGraph graph(renderer.Device());

    graph.AddShaderPack(&pack);
    graph.Bake();

}

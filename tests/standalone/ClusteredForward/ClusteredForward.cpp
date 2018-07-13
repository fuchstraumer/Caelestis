#include "ClusteredForward.hpp"
#include "graph/RenderGraph.hpp"
#include "systems/BufferResourceCache.hpp"
#include "systems/ImageResourceCache.hpp"
#include "core/ShaderResource.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> args{ argv + 1, argv + argc };
    using namespace st;
    using namespace vpsk;

    const std::string working_dir_path("../tests/ClusteredForward/");
    ShaderPack pack(std::string(working_dir_path + "shaders/Pack.lua").c_str());

    RendererCore& renderer = RendererCore::GetRenderer();
    RenderGraph graph(renderer.Device());

}
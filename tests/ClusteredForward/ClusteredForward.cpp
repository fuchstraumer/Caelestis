#include "ClusteredForward.hpp"
#include "graph/RenderGraph.hpp"
int main(int argc, char* argv[]) {
    std::vector<std::string> args{ argv + 1, argv + argc };
    using namespace st;
    using namespace vpsk;

    const std::string working_dir_path("../tests/ClusteredForward/");
    ShaderPack pack(std::string(working_dir_path + "shaders/Pack.lua").c_str());

    RendererCore& renderer = RendererCore::GetRenderer();
    RenderGraph graph(renderer.Device());

    std::vector<std::string> resource_group_names;
    {
        dll_retrieved_strings_t names = pack.GetResourceGroupNames();
        for (size_t i = 0; i < names.NumStrings; ++i) {
            resource_group_names.emplace_back(names.Strings[i]);
        }
    }

    

}
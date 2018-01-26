#include <iostream>
#include <experimental/filesystem>
#include "render/VpskRenderpass.hpp"
int main(int argc, char* argv[]) {
    namespace fs = std::experimental::filesystem;

    fs::path json("../rsrc/json/deferred.json");
    if (!fs::exists(json)) {
        throw std::runtime_error("Invalid path.");
    }

    using namespace vpsk;
    RenderPass pass;
    pass.LoadFromJSON(json.string());

    return 0;
}
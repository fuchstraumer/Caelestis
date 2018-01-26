#include "render/VpskRenderpass.hpp"
#include "third_party/json/src/json.hpp"
#include <fstream>

namespace vpsk {
    RenderPass::RenderPass(const vpr::Device * dvc) : device(dvc) {}

    void RenderPass::LoadFromJSON(const std::string& fname) {
        std::ifstream input_file(fname);
        if (!input_file.is_open()) {
            throw std::runtime_error("Failed to open input JSON file describing a renderpass.");
        }

        nlohmann::json j;
        input_file >> j;

        
    }

}
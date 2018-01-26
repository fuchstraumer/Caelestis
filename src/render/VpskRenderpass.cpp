#include "render/VpskRenderpass.hpp"
#include "core/LogicalDevice.hpp"
#include "util/EnumStringConverters.hpp"
#include "third_party/json/src/json.hpp"
#include <fstream>
#include <iostream>

namespace vpsk {

    RenderPass::RenderPass(const vpr::Device * dvc) : device(dvc) {
        if (device) {
            depthFormat = device->FindDepthFormat();
        }
    }

    void RenderPass::LoadFromJSON(const std::string& fname, const std::string& renderpass_name) {
        std::ifstream input_file(fname);
        if (!input_file.is_open()) {
            throw std::runtime_error("Failed to open input JSON file describing a renderpass.");
        }

        nlohmann::json j;
        input_file >> j;
        if (j.count(renderpass_name) == 0) {
            throw std::runtime_error("Renderpass name did not match input file.");
        }

        j = j.at(renderpass_name);
        try {
            auto attachments = j.at("color-attachments");
            for (auto iter = attachments.cbegin(); iter != attachments.cend(); ++iter) {
                const std::string attachment_name = iter.key();
                auto& resource = AddImageResource(iter->at("idx"), attachment_name);
                if (attachment_name == "depth-stencil") {
                    resource.Info.Format = depthFormat;
                }
                else if (attachment_name == "backbuffer") {
                    resource.Info.Format = swapchainFormat;
                }
                else {
                    resource.Info.Format = FormatFromStr(iter->at("format"), true);
                }
                resource.Info.Samples = iter->at("samples");
                resource.Info.finalLayout = LayoutFromStr(iter->at("finalLayout"), true);
                resource.Info.loadOp =  LoadOpFromStr(iter->at("loadOp"), true);
                resource.Info.storeOp = StoreOpFromStr(iter->at("storeOp"), true);
            }
            auto subpasses = j.at("subpasses");
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            throw e;
        }
        
        std::cerr << "done\n";
    }

    ImageResource & RenderPass::AddImageResource(const size_t & idx, const std::string & name) {
        auto iter = resources.insert(std::make_pair(name, ImageResource(idx, name)));
        return std::get<ImageResource>(resources.at(name));
    }

}
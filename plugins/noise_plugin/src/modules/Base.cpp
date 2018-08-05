#include "modules/Base.hpp"
#include "image/Image.hpp"
#include "ResourceContextAPI.hpp"
#include "ResourceTypes.hpp"
#include <vulkan/vulkan.h>
#include <algorithm>

namespace cnoise {
    
    Module::Module(const ResourceContext_API* rsrc_api, const size_t& width, const size_t& height) : dims(width, height), Generated(false), resourceApi(rsrc_api) {
        const VkBufferCreateInfo buffer_info{
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            nullptr,
            0,
            sizeof(float) * width * height,
            VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr
        };
        const VkBufferViewCreateInfo view_info{
            VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            nullptr,
            0,
            VK_NULL_HANDLE,
            VK_FORMAT_R32_SFLOAT,
            0,
            sizeof(float) * width * height
        };
        output = resourceApi->CreateBuffer(&buffer_info, &view_info, 0, nullptr, uint32_t(memory_type::DEVICE_LOCAL), nullptr);
    }

    Module::~Module() {
        resourceApi->DestroyResource(output);
    }

    void Module::ConnectModule(const std::shared_ptr<Module>& other) {
        if (sourceModules.size() < GetSourceModuleCount()) {
            sourceModules.push_back(other);
        }
    }

    std::vector<float> Module::GetData() const{
    }

    const Module* Module::GetModule(size_t idx) const {
        // .at(idx) has bounds checking in debug modes, iirc.
        return sourceModules[idx].get();
    }

    std::vector<float> Module::GetDataNormalized(float upper_bound, float lower_bound) const {
        {
            auto var = std::vector<float>{};
            auto min_max = std::minmax_element(var.cbegin(), var.cend());
            const float& min = *min_max.first;
            const float& max = *min_max.second;
            std::vector<float> result(var.data.cbegin(), var.data.cend());
            for(auto& elem : result) {
                elem = (elem - min) / (max - min);
            }
            return result;
        }
    }
    
    void Module::SaveToPNG(const char * name){
        std::vector<float> rawData = GetData();
        img::ImageWriter out(static_cast<int>(dims.first), static_cast<int>(dims.second));
        out.SetRawData(rawData);
        out.WritePNG(name);
    }

    void Module::SaveToPNG_16(const char * filename) {
        std::vector<float> raw = GetData();
        img::ImageWriter out(static_cast<int>(dims.first), static_cast<int>(dims.second));
        out.SetRawData(raw);
        out.WritePNG_16(filename);
    }

    void Module::SaveRaw32(const char* filename) {
        std::vector<float> raw = GetData();
        img::ImageWriter out(static_cast<int>(dims.first), static_cast<int>(dims.second));
        out.SetRawData(raw);
        out.WriteRaw32(filename);
    }

    void Module::SaveToTER(const char * name) {
        std::vector<float> rawData = GetData();
        img::ImageWriter out(static_cast<int>(dims.first), static_cast<int>(dims.second));
        out.SetRawData(rawData);
        out.WriteTER(name);
    }

    float* Module::GetDataPtr() {
        
    }

    const size_t & Module::Width() const noexcept {
        return dims.first;
    }

    const size_t & Module::Height() const noexcept {
        return dims.second;
    }

    void Module::checkSourceModules() {
        for (const auto m : sourceModules) {
            if (m == nullptr) {
                throw std::runtime_error("Source module in sourceModules was nullptr!");
            }
            if (!m->Generated) {
                m->Generate();
            }
        }
    }

}


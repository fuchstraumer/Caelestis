#pragma once
#ifndef VPSK_MATERIAL_SYSTEM_HPP
#define VPSK_MATERIAL_SYSTEM_HPP
#include "ForwardDecl.hpp"
#include "components/MaterialComponent.hpp"
#include "resources/Texture.hpp"
#include <unordered_map>
#include <string>
#include <memory>

namespace vpsk {

    class MaterialSystem {
        MaterialSystem(const MaterialSystem&) = delete;
        MaterialSystem& operator=(const MaterialSystem&) = delete;
    public:

    private:

        class materialData {
            materialData(const materialData&) = delete;
            materialData& operator=(const materialData&) = delete;
        public:
            
        private:

        };

        std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
        std::unordered_map<std::string, std::unique_ptr<vpr::Sampler>> samplers;

    };

}

#endif //!VPSK_MATERIAL_SYSTEM_HPP
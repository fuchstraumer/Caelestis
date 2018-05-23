#pragma once
#ifndef VPSK_SHADER_GROUP_HPP
#define VPSK_SHADER_GROUP_HPP
#include "ForwardDecl.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace vpsk {

    class ShaderGroup {
    public:
        


    private:
        std::vector<vpr::ShaderModule*> modules;
        VkShaderStageFlagBits stages;

    };

}

#endif //!VPSK_SHADER_GROUP_HPP
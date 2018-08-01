#pragma once
#ifndef NOISE_PLUGIN_PIPELINE_HASH_HPP
#define NOISE_PLUGIN_PIPELINE_HASH_HPP
#include <vulkan/vulkan.h>

/*

    Instead of always creating new pipelines for each module,
    we check a hashmap to see if a compatible pipeline already 
    exists. The dependent variables we need to ensure parity 
    with are:

    - specialization constants
    - the shader being used, of course
    - any state flags

    The latter shouldn't be an issue, as we will only be using
    compute pipelines

*/

size_t HashPipelineComponents(const char* shader_name, const VkPipelineLayout layout, const VkSpecializationInfo* info);

#endif //!NOISE_PLUGIN_PIPELINE_HASH_HPP
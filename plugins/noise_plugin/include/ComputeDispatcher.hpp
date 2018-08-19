#pragma once
#ifndef NOISE_PLUGIN_COMPUTE_DISPATCHER_HPP
#define NOISE_PLUGIN_COMPUTE_DISPATCHER_HPP
#include "NoiseModule.hpp"
#include <vulkan/vulkan.h>
#include <cstdint>
#include <cstddef>

struct RendererContext_API;
struct ResourceContext_API;

class ComputeDispatcher {
    ComputeDispatcher();
    ~ComputeDispatcher();
    ComputeDispatcher(const ComputeDispatcher&) = delete;
    ComputeDispatcher& operator=(const ComputeDispatcher&) = delete;
public:

    void Construct(const RendererContext_API* renderer_api, const ResourceContext_API* resource_api);
    void Destroy();

    NoiseModule* CreateModule(uint32_t type, uint32_t height, uint32_t width);
    void DestroyModule(NoiseModule* module);

    void SetModuleParameter(uint64_t module_handle, const char* parameter_name, void* parameter_value);
    void SetModuleConnection(uint64_t module_handle, const char* connection_type_name, uint64_t connecting_module);

    static ComputeDispatcher& GetDispatcher();

private:
};

#endif // !NOISE_PLUGIN_COMPUTE_DISPATCHER_HPP

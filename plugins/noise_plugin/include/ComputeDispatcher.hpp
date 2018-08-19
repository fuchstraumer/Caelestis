#pragma once
#ifndef NOISE_PLUGIN_COMPUTE_DISPATCHER_HPP
#define NOISE_PLUGIN_COMPUTE_DISPATCHER_HPP
#include <cstdint>
#include <cstddef>

struct RendererContext_API;
struct ResourceContext_API;
struct NoiseModule;

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

    static ComputeDispatcher& GetDispatcher();

private:

};

#endif // !NOISE_PLUGIN_COMPUTE_DISPATCHER_HPP

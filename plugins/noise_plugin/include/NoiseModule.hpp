#pragma once
#ifndef NOISE_PLUGIN_NOISE_MODULE_PUBLIC_INTERFACE_HPP
#define NOISE_PLUGIN_NOISE_MODULE_PUBLIC_INTERFACE_HPP
#include <cstdint>
#include <cstddef>

/*
    The following structure is how clients across the DLL
    see noise modules, while the full-fledged modules
    are stored in the dispatcher. This makes it possible
    to interact fully with the various modules without needing
    DLL linkage.
*/

struct NoiseModule {
    uint64_t Handle;
    uint32_t Type;
    uint32_t Width;
    uint32_t Height;
    uint64_t* Inputs;
    uint32_t NumInputs;
    void** Parameters;
    uint32_t NumParameters;
};

#endif //!NOISE_PLUGIN_NOISE_MODULE_PUBLIC_INTERFACE_HPP
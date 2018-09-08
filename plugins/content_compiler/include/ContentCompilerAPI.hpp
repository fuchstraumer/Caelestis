#pragma once 
#ifndef CONTENT_COMPILER_API_HPP
#define CONTENT_COMPILER_API_HPP

/*
    Function ideas and general inspiration for this plugin from:
    https://github.com/google/filament/tree/master/tools/filamesh -> Standardization, processing
    https://github.com/GPUOpen-Tools/Compressonator/blob/master/Compressonator/Source/CMP_MeshOptimizer/meshoptimizer.h -> Optimization
*/

constexpr static unsigned int ASSET_PIPELINE_PLUGIN_API_ID = 0x3e25cf4c;

struct MeshProcessingOptions;

struct ContentCompiler_API {
    // Blocking load
    struct MeshData* (*LoadMeshFromFileAssimp)(const char* fname, MeshProcessingOptions* options);
    // Signal called for async loading. Use instance pointer for a class instance,
    // if the signal function desired is a class member function (no *this with C)
    using mesh_loaded_signal_t = void(*)(void* instance, void* mesh_data);
    // Generated mesh data will use uninterleaved format
    void (*AsyncLoadMeshFromFileAssimp)(const char* fname, bool interleaved, void* requester, mesh_loaded_signal_t signal, MeshProcessingOptions* opts);
    void (*DestroyMeshData)(struct MeshData* data);
};

#endif //!CONTENT_COMPILER_API_HPP

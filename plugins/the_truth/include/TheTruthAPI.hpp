#pragma once
#ifndef CAELESTIS_THE_TRUTH_PLUGIN_API_HPP
#define CAELESTIS_THE_TRUTH_PLUGIN_API_HPP
#include <cstdint>

constexpr static uint32_t RESOURCE_CONTEXT_API_ID = 0x284a326b;

struct TruthInstance;

enum class truth_type : uint16_t {
    INT8 = 0u,
    UINT8 = 1u,
    BOOL = 2u,
    INT16 = 3u,
    UINT16 = 4u,
    INT32 = 5u,
    UINT32 = 6u,
    INT64 = 7u,
    UINT64 = 8u,
    FLOAT = 9u,
    DOUBLE = 10u,
    LONG_DOUBLE = 11u,
    STRING = 12u,
    BUFFER = 13u,
    OBJECT_REFERENCE = 14u,
    SUBOBJECT = 15u
};

struct TheTruthAPI {
    uint64_t (*CreateObject)(TruthInstance* ti);
    void (*DestroyObject)(TruthInstance* ti, uint64_t obj);
    uint64_t (*CreateObjectOfType)(TruthInstance* ti, truth_type type);
};

#endif //!CAELESTIS_THE_TRUTH_PLUGIN_API_HPP

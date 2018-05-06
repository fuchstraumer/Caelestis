#pragma once

#ifndef NDEBUG
#define ENTITY_SYSTEM_STRICT_CHECKS
#endif // !NDEBUG


// Need to move this config to CMake eventually
#ifdef ENTITY_SYSTEM_STRICT_CHECKS
constexpr bool ENTITY_SYSTEM_CHECKS = true;
#else
constexpr bool ENTITY_SYSTEM_CHECKS = false;
#endif
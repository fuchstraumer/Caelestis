#pragma once
#ifndef CAELESTIS_ASSERT_HPP
#define CAELESTIS_ASSERT_HPP
#include "API.hpp"

#ifdef CA_ASSERT_ENABLED
void __cdecl CA_API void ca_assert(const wchar_t* message, const wchar_t* where, unsigned line);
#define caAssert(expression) (void)((!(expression)) || (ca_assert(#expression, __FILE__, __LINE__)))
#else
#define caAssert(expression) ((void)0)
#endif

#endif //!CAELESTIS_ASSERT_HPP

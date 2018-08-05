#pragma once
#ifndef MODULES_H
#define MODULES_H

// Include base so its easier to attach modules and such
#include "Base.hpp"

// Include modules.
#include "combiners\Combiners.hpp"
#include "generators\Generators.hpp"
#include "modifiers\Modifiers.hpp"
#include "utility\Utility.hpp"

// Simple macro to shortcut away the namespaces.
#ifdef USING_CNOISE_NAMESPACES
using namespace cnoise::combiners;
using namespace cnoise::generators;
using namespace cnoise::modifiers;
#endif // USING_CNOISE_NAMESPACES


#endif // !MODULES_H

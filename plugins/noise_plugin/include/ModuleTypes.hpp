#pragma once
#ifndef NOISE_PLUGIN_MODULE_TYPES_HPP
#define NOISE_PLUGIN_MODULE_TYPES_HPP

enum CombinerModules : unsigned int {
    Invalid = 0,
    Add = 1,
    Blend = 2,
    Divide = 3,
    Max = 4,
    Min = 5,
    Multiply = 6,
    Power = 7,
    Select = 8,
    Subtract = 9,
    EndRange = 10
};

enum ModifierModules : unsigned int {
    Invalid = 0,
    Abs = 1,
    Clamp = 2,
    Curve = 3,
    Downsample = 4,
    ScaleBias = 5,
    Terrace = 6,
    EndRange = 7, 
};

enum GeneratorModules : unsigned int {
    Invalid = 0,
    Billow = 1,
    Decarpientier = 2,
    FBM = 3,
    Jordan = 4,
    RidgedMulti = 5,
    EndRange = 6,
};

enum UtilityModules : unsigned int {
    Invalid = 0,
    Cache = 1,
    Checkerboard = 2,
    EndRange = 3
};

#endif //!NOISE_PLUGIN_MODULE_TYPES_HPP


local dimensions = require("Functions")

ObjectSizes = {
    Material = 128
}

Resources = {
    GlobalResources = {
        UBO = {
            Type = "UniformBuffer",
            Members = {
                model = { "mat4", 0 },
                view = { "mat4", 1 },
                projectionClip = { "mat4", 2 },
                normal = { "mat4", 3 },
                viewPosition = { "vec4", 4 },
                depth = { "vec2", 5 },
                numLights = { "uint", 6 }
            }
        }
    },
    ClusteredForward = {
        flags = {
            Type = "StorageTexelBuffer",
            Format = "r8ui",
            Size = dimensions.TotalTileCount(),
            Qualifiers = "restrict"
        },
        bounds = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = NumLights * 6,
            Qualifiers = "restrict"
        },
        lightCounts = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = dimensions.TotalTileCount(),
            Qualifiers = "restrict"
        },
        lightCountTotal = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = 1,
            Qualifers = "restrict"
        },
        lightCountOffsets = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = dimensions.TotalTileCount(),
            Qualifers = "restrict"
        },
        lightList = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = 1024 * 1024,
            Qualifers = "restrict"
        },
        lightCountCompare = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = dimensions.TotalTileCount(),
            Qualifers = "restrict"
        }
    },
    Lights = {
        positionRanges = {
            Type = "StorageTexelBuffer",
            Format = "rgba32f",
            Size = dimensions.NumLights,
            Qualifiers = "restrict readonly"
        }
        lightColors = {
            Type = "UniformTexelBuffer",
            Format = "rgba8",
            Size = dimensions.NumLights,
            Qualifiers = "restrict readonly"
        }
    },
    ObjMaterial = {
        Material = {
            Type = "UniformBuffer",
            Members = {
                ambient = { "vec4", 0 },
                diffuse = { "vec4", 1 },
                specular = { "vec4", 2 },
                transmittance = { "vec4", 3 },
                emission = { "vec4", 4 },
                shininess = { "float", 5 },
                ior = { "float", 6 },
                alpha = { "float", 7 },
                illuminationModel = { "int", 8 },
                roughness = { "float", 9 },
                metallic = { "float", 10 },
                sheen = { "float", 11 },
                clearcoatThickness = { "float", 12 },
                clearcoatRoughness = { "float", 13 },
                anisotropy = { "float", 14 },
                anisotropyRotation = { "float", 15 },
                padding = { "float", 16 }
            }
        },
        diffuseMap = {
            Type = "CombinedImageSampler",
            FromFile = true
        },
        normalMap = {
            Type = "CombinedImageSampler",
            FromFile = true
        },
        roughnessMap = {
            Type = "CombinedImageSampler",
            FromFile = true
        },
        metallicMap = {
            Type = "CombinedImageSampler",
            FromFile = true
        }
    }
}
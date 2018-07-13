
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
        Flags = {
            Type = "StorageTexelBuffer",
            Format = "r8ui",
            Size = dimensions.TotalTileCount(),
            Qualifiers = "restrict"
        },
        lightBounds = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = dimensions.NumLights * 6,
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
            Qualifiers = "restrict"
        },
        lightCountOffsets = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = dimensions.TotalTileCount(),
            Qualifiers = "restrict"
        },
        lightList = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = 1024 * 1024,
            Qualifiers = "restrict"
        },
        lightCountCompare = {
            Type = "StorageTexelBuffer",
            Format = "r32ui",
            Size = dimensions.TotalTileCount(),
            Qualifiers = "restrict"
        },
        -- Tags can be used by frontends to specify certain behavior.
        -- Here, I'll use these tags to know that this group has resources that 
        -- should be zero-initialized and cleared per pass/execution run
        Tags = { "InitializeToZero", "ClearAtEndOfPass" }
    },
    Lights = {
        positionRanges = {
            Type = "StorageTexelBuffer",
            Format = "rgba32f",
            Size = dimensions.NumLights,
            Qualifiers = "restrict",
            Tags = { "HostGeneratedData" }
        },
        lightColors = {
            Type = "StorageTexelBuffer",
            Format = "rgba8",
            Size = dimensions.NumLights,
            Qualifiers = "restrict readonly",
            Tags = { "HostGeneratedData" }
        }
    },
    ObjMaterials = {
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
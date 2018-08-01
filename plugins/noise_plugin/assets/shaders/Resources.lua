
local cfg = require("NoiseConfig")

Resources = {
    GeneratorTextures = {
        permTex = {
            Type = "CombinedImageSampler",
            ImageInfo = {
                ImageType = "2D",
                FromFile = true
            },
            ViewInfo = {
                ViewType = "2D",
                Aspect = "Color"
            },
            SamplerInfo = {
                MagFilter = "Nearest",
                MinFilter = "Nearest",
                AddressModeU = "Repeat",
                AddressModeV = "Repeat"
                EnabledAnisotropy = false
            }
        },
        gradTex = {
            Type = "CombinedImageSampler",
            ImageInfo = {
                ImageType = "2D",
                FromFile = true
            },
            ViewInfo = {
                ViewType = "2D",
                Aspect = "Color"
            },
            SamplerInfo = {
                MagFilter = "Nearest",
                MinFilter = "Nearest",
                AddressModeU = "Repeat",
                AddressModeV = "Repeat"
                EnabledAnisotropy = false
            }
        }
    },
    BaseResources = {
        output = {
            Type = "StorageTexelBuffer",
            Format = cfg.OutputFormat(),
            Size = cfg.TexelBufferSize()
        },
        input0 = {
            Type = "StorageTexelBuffer",
            Format = cfg.InputFormat(),
            Size = cfg.TexelBufferSize()
        },
        input1 = {
            Type = "StorageTexelBuffer",
            Format = cfg.InputFormat(),
            Size = cfg.TexelBufferSize()
        },
        positions = {
            Type = "StorageTexelBuffer",
            Format = cfg.PositionsFormat(),
            Size = cfg.TexelBufferSize()
        }
    }
}

local dimensions = require("cfDimensions.lua");

Config = {
    FeatureName = "ClusteredForward",
    Description = [[
        Uses the Clustered Forward rendering technique
        to perform the core rendering of objects, using
        specified lights. Only supports spot lights, currently
    ]],
    ShaderPack = "ClusteredForward",
    cvars = {
        { "ResolutionX", GetScreenX() },
        { "ResolutionY", GetScreenY() },
        { "MinLights", 1024 },
        { "MaxLights", 4096 },
        { "NumLights", dimensions.NumLights() },
        { "GenerateLights", false },
        { "TileWidth", 64 },
        { "TileHeight", 64 }
    },
    PipelineSubmissions = {
        { 0, "DepthPrePass" },
        { 1, "LightingOpaque" },
        { 2, "LightingTransparent" },
        { 3, "ComputeLightGrids" },
        { 4, "ComputeGridOffsets" },
        { 5, "ComputeLightList" },
        { 6, "OpaqueOnscreenPass" },
        { 7, "TransparentOnscreenPass" }
    }
}

PipelineConfigs = {
    DepthPrePass = {
        ColorBlendInfo = {
            AttachmentCount = 0
        },
        ShaderGroupName = "DepthPrePass"
    },
    LightingOpaque = {
        DepthStencilInfo = {
            DepthWriteEnable = false,
        },
        ColorBlendInfo = {
            AttachmentCount = 0
        },
        DynamicStateInfo = {
            DynamicStateCount = 2,
            DynamicStates = {
                "Viewport", "Scissor"
            }
        },
        ShaderGroupName = "Lights"
    },
    LightingTransparent = {
        DepthStencilInfo = {
            DepthWriteEnable = false,
        },
        ColorBlendInfo = {
            AttachmentCount = 0
        },
        RasterizationInfo = {
            CullMode = "None"
        },
        DynamicStateInfo = {
            DynamicStateCount = 2,
            DynamicStates = {
                "Viewport", "Scissor"
            }
        },
        ShaderGroupName = "Lights",
        BasePipelineName = "LightingOpaque"
    },
    --[[
        In the case of compute shaders, we don't have any real pipeline
        info to explain or describe.

        If the script parser/execution system can't find a pipeline info
        entry for one of the listed submissions, it assumes it's creating
        a compute PipelineSubmission and looks to match the PipelineSubmission's
        name to one of the available shadergroup names

        From there, we can create a VkPipleline object for that compute pipeline
    ]]
    OpaqueOnscreenPass = {
        ColorBlendInfo = {
            AttachmentCount = 1,
            Attachments = {
                {0, {
                    BlendEnable = false,
                    -- Use defaults for rest
                }}
            }
        }, 
        DynamicStateInfo = {
            DynamicStateCount = 2,
            DynamicStates = {
                "Viewport", "Scissor"
            }
        },
        ShaderGroupName = "Clustered"
    },
    TransparentOnscreenPass = {
        DepthStencilInfo = {
            DepthWriteEnable = false
        },
        -- Default value for color blend info works for the
        -- transparent pass. Just had to disable blending for
        -- the opaque pass.
        RasterizationInfo = {
            CullMode = "None"
        },
        DynamicStateInfo = {
            DynamicStateCount = 2,
            DynamicStates = {
                "Viewport", "Scissor"
            }
        },
        ShaderGroupName = "Clustered",
        BasePipelineName = "OpaqueOnscreenPass"
    }
}
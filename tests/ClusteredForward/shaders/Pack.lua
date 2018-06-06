PackName = "ClusteredForwardShading"
ResourceFileName = "Resources.lua"
ShaderGroups = {
    DepthPrePass = { 
        Idx = 0,
        Shaders = { Vertex = "Simple.vert" },
        Tags = { "DepthOnly", "PrePass" }
    },
    Lights = { 
        Idx = 1,
        Shaders = { Vertex = "Default.vert", Fragment = "Light.frag" }
    },
    ComputeLightGrids = { 
        Idx = 2,
        Shaders = { Compute = "LightGrid.comp" }
    },
    ComputeGridOffsets = { 
        Idx = 3,
        Shaders = { Compute = "GridOffsets.comp" }
    },
    ComputeLightLists = { 
        Idx = 4,
        Shaders = { Compute = "LightList.comp" }
    },
    Clustered = { 
        Idx = 5,
        Shaders = { Vertex = "Default.vert", Fragment = "Clustered.frag" }
    }
}

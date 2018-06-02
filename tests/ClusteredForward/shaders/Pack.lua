PackName = "ClusteredForwardShading"
ResourceFileName = "Resources.lua"
ShaderGroups = {
    DepthPrePass = {
        Vertex = "Simple.vert"
    },
    Clustered = {
        Vertex = "Default.vert",
        Fragment = "Clustered.frag"
    },
    Lights = {
        Vertex = "Default.vert",
        Fragment = "Light.frag"
    },
    ComputeLightGrids = {
        Compute = "LightGrid.comp"
    },
    ComputeGridOffsets = {
        Compute = "GridOffsets.comp"
    },
    ComputeLightLists = {
        Compute = "LightList.comp"
    }
}
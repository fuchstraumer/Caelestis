PackName = "ClusteredForwardShading"
ResourceFileName = "Resources.lua"
ShaderGroups = {
    DepthPrePass = { 0, {
        Vertex = "Simple.vert"
    } },
    Lights = { 1, {
        Vertex = "Default.vert",
        Fragment = "Light.frag"
    } },
    ComputeLightGrids = { 2, {
        Compute = "LightGrid.comp"
    } },
    ComputeGridOffsets = { 3, {
        Compute = "GridOffsets.comp"
    } },
    ComputeLightLists = { 4, {
        Compute = "LightList.comp"
    } },
    Clustered = { 5, {
        Vertex = "Default.vert",
        Fragment = "Clustered.frag"
    } }
}
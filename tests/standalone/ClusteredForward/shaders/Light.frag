layout(early_fragment_tests) in;
SPC const uint ResolutionX = 1920;
SPC const uint ResolutionY = 1080;
SPC const uint TileCountX = (1920 - 1) / 64 + 1;
SPC const uint TileCountY = (1080 - 1) / 64 + 1;
SPC const uint TileCountZ = 256;
SPC const uint TileWidth = 64;
SPC const uint TileHeight = 64;
#pragma USE_RESOURCES GlobalResources
#pragma USE_RESOURCES ClusteredForward

uvec3 viewPosToGrid(vec2 frag_pos, float view_z) {
    vec3 c;
    c.xy = (frag_pos - 0.50f) / vec2(float(TileWidth),float(TileHeight));
    c.z = min(float(TileCountZ - 1), max(0.0f, float(TileCountZ) * log((-view_z - UBO.depth.x) / (UBO.depth.y - UBO.depth.x) + 1.0f)));
    return uvec3(c);
}

uint CoordToIdx(uvec3 c) {
    return TileCountX * TileCountY * c.z + TileCountX * c.y + c.x;
}

void main() {
    vec4 vpos = UBO.view * vec4(vPosition, 1.0f);
    uint idx = CoordToIdx(viewPosToGrid(gl_FragCoord.xy, vpos.z));
    imageStore(Flags, int(idx), uvec4(1,0,0,0));
}

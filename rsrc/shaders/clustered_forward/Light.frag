#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;
layout(location = 0) in vec4 vPosition;

layout (constant_id =  0) const uint ResolutionX = 1920;
layout (constant_id =  1) const uint ResolutionY = 1080;
// TileSize vector in sample code
layout (constant_id =  2) const uint TileCountX = (1920 - 1) / 64 + 1;
layout (constant_id =  3) const uint TileCountY = (1080 - 1) / 64 + 1;
layout (constant_id =  4) const uint TileWidth = 64;
layout (constant_id =  5) const uint TileHeight = 64;
layout (constant_id =  6) const uint TileCountZ = 256;

layout (set = 0, binding = 0) uniform _ubo {
    mat4 model;
    mat4 view;
    mat4 projectionClip;
    mat4 normal;
    vec4 viewPosition;
    vec2 depth;
    uint numLights;
} UBO;

layout (set = 1, binding = 0, r8ui) uniform uimageBuffer Flags;

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
    vec4 vpos = UBO.view * vPosition;
    uint idx = CoordToIdx(viewPosToGrid(gl_FragCoord.xy, vpos.z));
    imageStore(Flags, int(idx), uvec4(1,0,0,0));
}
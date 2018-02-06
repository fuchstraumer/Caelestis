#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;
layout(location = 0) in vec4 vPosition;

layout(constant_index = 0) const uint TileCountZ = 256;
layout(constant_index = 1) const uint LightListMax = 512;
layout(constant_index = 2) const uint ResolutionX = 1440;
layout(constant_index = 3) const uint ResolutionY = 900;
// TileSize vector in sample code
layout(constant_index = 4) const uint TileWidthX = 64;
layout(constant_index = 5) const uint TileWidthY = 64;
layout(constant_index = 6) const float NearPlane = 0.1f;
layout(constant_index = 7) const float FarPlane = 3000.0f;
// Grid dimension vector in sampler code.
layout(constant_index = 8) const uint TileCountX = (ResolutionX - 1) / (TileWidthX + 1);
layout(constant_index = 9) const uint TileCountY = (ResolutionY - 1) / (TileWidthY + 1);

layout (set = 0, binding = 0) uniform _ubo {
    mat4 model;
    mat4 view;
    mat4 projectionClip;
    mat4 normal;
    vec4 viewPosition;
    uint numLights;
} UBO;

layout (set = 1, binding = 0, r8ui) uniform uimageBuffer Flags;


uvec3 viewPosToGrid(vec2 frag_pos, float view_z) {
    vec3 c(frag_pos / vec2(float(TileWidthX),float(TileWidthY)), 1.0f);
    c.z = min(float(TileCountZ - 1), max(0.f, float(TileCountZ) * log((-view_z - NearPlane) / (FarPlane - NearPlane) + 1.0f)));
    return uvec3(c);
}

uint CoordToIdx(uvec3 c) {
    return TileCountX * TileCountY * c.z + TileCountX * c.y + c.x;
}

void main() {
    vec4 vpos = UBO.View * vPosition;
    uint idx = CoordToIdx(viewPosToGrid(gl_FragCoord.xy, vpos.z));
    imageStore(Flags, int(idx), uvec4(1,0,0,0));
}
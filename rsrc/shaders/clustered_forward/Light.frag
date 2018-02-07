#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;
layout(location = 0) in vec4 vPosition;
layout (constant_id =  0) const uint ResolutionX = 1440;
layout (constant_id =  1) const uint ResolutionY = 900;
layout (constant_id =  2) const uint LightListMax = 512;
// TileSize vector in sample code
layout (constant_id =  3) const uint TileWidthX = 64;
layout (constant_id =  4) const uint TileWidthY = 64;
// Grid dimension vector in sampler code.
layout (constant_id =  5) const uint TileCountX = (ResolutionX - 1) / (TileWidthX + 1);
layout (constant_id =  6) const uint TileCountY = (ResolutionY - 1) / (TileWidthY + 1);
layout (constant_id =  7) const uint TileCountZ = 256;
layout (constant_id =  8) const float NearPlane = 0.1f;
layout (constant_id =  9) const float FarPlane = 3000.0f;
layout (constant_id = 10) const float AmbientGlobal = 0.20f;


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
    vec3 c;
    c.xy = frag_pos / vec2(float(TileWidthX),float(TileWidthY));
    c.z = min(float(TileCountZ - 1), max(0.f, float(TileCountZ) * log((-view_z - NearPlane) / (FarPlane - NearPlane) + 1.0f)));
    return uvec3(c);
}

int CoordToIdx(uvec3 c) {
    return int(TileCountX * TileCountY * c.z + TileCountX * c.y + c.x);
}

void main() {
    vec4 vpos = UBO.view * vPosition;
    int idx = CoordToIdx(viewPosToGrid(gl_FragCoord.xy, vpos.z));
    imageStore(Flags, int(idx), uvec4(1,0,0,0));
}
#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (constant_id =  0) const uint ResolutionX = 1920;
layout (constant_id =  1) const uint ResolutionY = 1080;
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


layout (location = 0) in vec4 posRangeIn;
layout (location = 1) in vec4 color;
layout (location = 0) out vec3 vColor;

layout (set = 0, binding = 0) uniform _ubo {
    mat4 model;
    mat4 view;
    mat4 projectionClip;
    mat4 normal;
    vec4 viewPosition;
    uint numLights;
} UBO;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

void main() {
    vColor = color.xyz;
    gl_Position = UBO.projectionClip * UBO.view * vec4(posRangeIn.xyz, 1.0f);
    gl_PointSize = 0.20f * (FarPlane / gl_Position.z);
}


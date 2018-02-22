#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
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

layout (location = 0) in vec4 posRangeIn;
layout (location = 1) in vec4 color;
layout (location = 0) out vec3 vColor;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

void main() {
    vColor = color.xyz;
    gl_Position = UBO.projectionClip * UBO.view * vec4(posRangeIn.xyz, 1.0f);
    gl_PointSize = 2.0f * (UBO.depth.y / gl_Position.z);
}


#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform colors {
    layout(offset=80) vec4 color;
} ubo;

void main() {
    fragColor = ubo.color;
}
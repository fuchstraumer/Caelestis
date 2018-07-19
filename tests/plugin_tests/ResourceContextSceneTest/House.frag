#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 backbuffer;

layout (set = 0, binding = 1) uniform sampler2D diffuse;

void main() {
    backbuffer = texture(diffuse, vUV);
}

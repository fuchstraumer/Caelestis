#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 backbuffer;

layout (set = 1, binding = 0) uniform sampler texSampler;
layout (set = 0, binding = 1) uniform texture2D diffuse;

void main() {
    backbuffer = texture(sampler2D(diffuse, texSampler), vUV);
}

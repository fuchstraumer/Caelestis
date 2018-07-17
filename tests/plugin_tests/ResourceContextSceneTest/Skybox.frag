#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 vUV;
layout (location = 0) out vec4 backbuffer;

layout (set = 1, binding = 0) uniform sampler texSampler;
layout (set = 0, binding = 1) uniform textureCube skybox;

void main() {
    backbuffer = texture(samplerCube(skybox, texSampler), vUV);
}

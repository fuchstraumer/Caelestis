#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 vUV;
layout (location = 0) out vec4 backbuffer;

layout (set = 0, binding = 1) uniform samplerCube skybox;

void main() {
    backbuffer = texture(skybox, vUV);
}

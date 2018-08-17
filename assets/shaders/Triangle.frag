#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 vColor;
layout (location = 0) out vec4 backbuffer;

void main() {
    backbuffer = vec4(vColor, 1.0f);
}
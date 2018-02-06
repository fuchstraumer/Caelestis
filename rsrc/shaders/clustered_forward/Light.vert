#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 pos;
layout (location = 0) out vec4 vPosition;

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
};

void main() {
    vPosition = UBO.model * vec4(pos,1.0f);
    gl_Position = UBO.projectionClip * UBO.view * vPosition;
}
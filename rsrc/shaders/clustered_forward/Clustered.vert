#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec4 vPosition;
layout (location = 1) out vec4 vNormal;
layout (location = 2) out vec4 vTangent;
layout (location = 3) out vec2 vUV;

out gl_PerVertex {
    vec4 gl_Position;
};

layout (set = 1, binding = 0) uniform _ubo {
    mat4 model;
    mat4 view;
    mat4 projectionClip;
    mat4 normal;
    vec4 viewPosition;
    uint numLights;
} UBO;

void main() {
    vPosition = UBO.model * vec4(position, 1.0f);
    vNormal = UBO.normal * vec4(normal, 1.0f);
    vTangent = UBO.normal * vec4(tangent, 1.0f);
    vUV = uv;
    gl_Position = UBO.projectionClip * UBO.view * vPosition;
}
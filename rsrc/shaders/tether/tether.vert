#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 offset;
layout (location = 3) in vec3 color;

layout (location = 0) out vec3 vColor;

layout(push_constant) uniform _ubo {
    mat4 view;
    mat4 projection;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};


void main() {
    gl_Position = ubo.projection * ubo.view * vec4(position + offset, 1.0f);
    vColor = color;
}
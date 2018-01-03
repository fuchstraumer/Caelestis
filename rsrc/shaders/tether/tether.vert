#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 offset;

layout(push_constant) uniform _ubo {
    mat4 view;
    mat4 projection;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = projection * view * vec4(position + offset, 1.0f);
}
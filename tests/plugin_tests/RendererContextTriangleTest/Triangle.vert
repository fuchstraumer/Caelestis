#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 vColor;

layout (set = 0, binding = 0) uniform __ubo__ {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(position, 1.0f);
    vColor = color;
}
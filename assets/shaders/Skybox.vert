#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 vUV;

layout (set = 0, binding = 0) uniform uniform_buffer {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 position = ubo.model * ubo.projection * ubo.view * vec4(pos, 1.0f);
    gl_Position = position.xyww;
    vUV = pos;
}

#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 pos;

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
    gl_Position = UBO.projectionClip * UBO.view * UBO.model * vec4(pos,1.0f);
}
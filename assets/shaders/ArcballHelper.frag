#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform push_color {
    layout (offset = 64) vec4 color;
} ubo;

layout (location = 0) out vec4 backbuffer;

void main() {
    backbuffer = ubo.color;
}

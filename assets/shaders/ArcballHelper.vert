#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;

layout (push_constant) uniform proj_view {
    mat4 projView;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.projView * vec4(position, 1.0f);
}

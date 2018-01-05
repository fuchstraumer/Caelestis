#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;

layout (location = 0) out vec3 vColor;
layout(location = 1) out vec3 vPos;
layout(location = 2) out vec3 vNorm;

layout(push_constant) uniform _ubo {
    mat4 view;
    mat4 projection;
} ubo;

layout (constant_id = 0) const uint INSTANCE_COUNT = 11;

layout(location = 0) uniform ubo2 {
    vec4 p[INSTANCE_COUNT];
} offsets;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 final_pos = vec4(position,1.0f) + offsets.p[gl_InstanceIndex];
    gl_Position = ubo.projection * ubo.view * final_pos;
    vColor = color;
    vPos = final_pos.xyz;
    vNorm = normal;
}
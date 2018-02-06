#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(constant_index = 6) const float NearPlane = 0.1f;
layout(constant_index = 7) const float FarPlane = 3000.0f;

layout (location = 0) in vec4 posRangeIn;
layout (lcoation = 1) in vec4 color;
layout (location = 0) out vec3 vColor;

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
    float gl_PointSize;
};

void main() {
    vColor = color.xyz;
    gl_Position = UBO.projectionClip * UBO.view * vec4(posRangeIn.xyz, 1.0f);
    gl_PointSize = 0.20f * (FarPlane / gl_Position.z);
}


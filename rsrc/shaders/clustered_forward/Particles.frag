#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 vColor;
layout (location = 0) out vec4 fragColor;

void main() {
    float r = distance(gl_PointCoord.xy, vec2(0.50f));
    if (r > 0.50f) {
        discard;
    }

    fragColor = vec4(vColor, 4.0f * pow(0.50f - r, 2.0f));
}
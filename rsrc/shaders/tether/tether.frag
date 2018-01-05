#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec3 vPos;
layout(location = 2) in vec3 vNorm;
layout(location = 0) out vec4 fragColor;

layout(location = 1) uniform fubo {
    vec4 viewerPos;
    vec4 lightPos;
} ubo;

void main() {
    fragColor = vec4(vColor, 1.0f);
    vec3 light_color = vec3(0.95f, 0.95f, 0.97f);
    const float ambient_strength = 0.3f;
    vec3 ambient = ambient_strength * light_color;

    const float diffuse_strength = 0.6f;
    vec3 light_direction = normalize(ubo.lightPos.xyz - vPos);
    float diff = max(dot(vNorm, light_direction), 0.0f);
    vec3 diffuse = diffuse_strength * diff * light_color;

    float specular_strength = 0.3f;
    vec3 view_direction = normalize(ubo.viewerPos.xyz - vPos);
    vec3 reflect_direction = reflect(-light_direction, vNorm);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0f), 32.0f);
    vec3 specular = specular_strength * spec * light_color;

    vec4 light_result = vec4(ambient + diffuse + specular, 1.0f);
    fragColor.w = 1.0f;
    fragColor.xyz = vColor;
    fragColor *= light_result;
}
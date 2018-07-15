#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (constant_id =  0) const uint ResolutionX = 1920;
layout (constant_id =  1) const uint ResolutionY = 1080;
// TileSize vector in sample code
layout (constant_id =  2) const uint TileCountX = (1920 - 1) / 64 + 1;
layout (constant_id =  3) const uint TileCountY = (1080 - 1) / 64 + 1;
layout (constant_id =  4) const uint TileWidth = 64;
layout (constant_id =  5) const uint TileHeight = 64;
layout (constant_id =  6) const uint TileCountZ = 256;

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vNormal;
layout (location = 2) in vec4 vTangent;
layout (location = 3) in vec2 vUV;

layout (location = 0) out vec4 fragColor; 

// Global uniforms
layout (set = 1, binding = 0) uniform _ubo {
    mat4 model;
    mat4 view;
    mat4 projectionClip;
    mat4 normal;
    vec4 viewPosition;
    vec2 depth;
    uint numLights;
} UBO;

// Clustered-forward technique uniforms
layout (set = 1, binding = 1, rgba32f) uniform readonly imageBuffer positionRanges;
layout (set = 1, binding = 2, rgba8) uniform readonly imageBuffer lightColors;
layout (set = 0, binding = 0, r8ui) uniform readonly uimageBuffer flags;
layout (set = 0, binding = 2, r32ui) uniform readonly uimageBuffer lightCounts;
layout (set = 0, binding = 4, r32ui) uniform readonly uimageBuffer lightCountOffsets;
layout (set = 0, binding = 5, r32ui) uniform readonly uimageBuffer lightList;

// Per-material uniforms
layout (set = 2, binding = 4) uniform _mtl {
    vec4 ambient;
    vec4 diffuse; 
    vec4 specular;
    vec4 transmittance;
    vec4 emission;
    layout (offset = 80) float shininess;
    layout (offset = 84) float ior;
    layout (offset = 88) float alpha;
    layout (offset = 92) int illuminationModel;
    layout (offset = 96) float roughness;
    layout (offset = 100) float metallic;
    layout (offset = 104) float sheen;
    layout (offset = 108) float clearcoatThickness;
    layout (offset = 112) float clearcoatRoughness;
    layout (offset = 116) float anisotropy;
    layout (offset = 120) float anisotropyRotation;
    layout (offset = 124) float padding;
} Material;

layout (set = 2, binding = 0) uniform sampler2D diffuseMap;
layout (set = 2, binding = 1) uniform sampler2D normalMap;
layout (set = 2, binding = 2) uniform sampler2D roughnessMap;
layout (set = 2, binding = 3) uniform sampler2D metallicMap;


uint CoordToIdx(uint i, uint j, uint k) {
    return TileCountX * TileCountY * k + TileCountX * j + i;
}

vec3 viewPosToGrid(vec2 frag_pos, float view_z) {
    vec3 c;
    c.xy = frag_pos / vec2(float(TileWidth),float(TileHeight));
    c.z = min(float(TileCountZ - 1), max(0.f, float(TileCountZ) * log((-view_z - UBO.depth.x) / (UBO.depth.y - UBO.depth.x) + 1.0f)));
    return c;
}

vec3 fresnelSchlick(float cos_theta, vec3 F0) {
    return F0 + (1.0f - F0) * pow(1.0f - cos_theta, 5.0f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    a *= a;
    float NdotH = max(dot(N,H),0.0f);
    NdotH *= NdotH;
    float denom = (NdotH * (a - 1.0f) + 1.0f);
    denom *= (3.14159f * denom);
    return a / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    return (NdotV * (1.0f - k) + k) / NdotV;
}

float GeometrySmith(vec3 n, vec3 v, vec3 l, float roughness) {
    float NdotV = max(dot(n,v),0.0f);
    float NdotL = max(dot(n,l),0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

void main() {

    vec3 normal_sample = texture(normalMap, vUV).rgb * vec3(2.0f) - vec3(1.0f);
    vec3 world_normal = mat3(UBO.normal) * normal_sample;

    const float metallic = texture(metallicMap, vUV).r;
    const float roughness = texture(roughnessMap, vUV).r;
    const vec3 albedo = texture(diffuseMap, vUV).rgb;
    const float ao = 1.0f;

    vec3 view_dir = normalize(UBO.viewPosition.xyz - vPosition.xyz);

    if (Material.alpha < 1.0f && dot(view_dir, world_normal) < 0.0f) {
        fragColor = vec4(0.0f);
        return;
    }

    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, albedo, metallic);

    vec3 view_pos = (UBO.view * vec4(vPosition.xyz,1.0f)).xyz;
    uvec3 grid_coord = uvec3(viewPosToGrid(gl_FragCoord.xy, view_pos.z));
    int grid_idx = int(CoordToIdx(grid_coord.x, grid_coord.y, grid_coord.z));

    vec3 lighting = vec3(0.0f);

    if (imageLoad(flags, grid_idx).r == 1) {
        uint offset = imageLoad(lightCountOffsets, grid_idx).r;
        uint light_count = imageLoad(lightCounts, grid_idx).r;
        for (uint i = 0; i < light_count; ++i) {
            int light_idx = int(imageLoad(lightList, int(offset + i).r));
            vec4 light_pos_range = imageLoad(positionRanges, light_idx);
            float dist = distance(light_pos_range.xyz, vPosition.xyz);
            if (dist < light_pos_range.w) {
                const vec3 light_color = imageLoad(lightColors, light_idx).rgb;
                const vec3 l = normalize(light_pos_range.xyz - vPosition.xyz);
                const vec3 h = normalize(view_dir + l);
                float atten = max(1.0f - max(0.0f, dist / light_pos_range.w), 0.0f);
                vec3 radiance = light_color * atten;
                
                float ndf = DistributionGGX(world_normal, h, roughness);
                float G = GeometrySmith(world_normal, view_dir, l, roughness);
                vec3 F = fresnelSchlick(max(dot(h, view_dir), 0.0f), F0);

                vec3 kd = vec3(1.0f) - F;
                kd *= 1.0f - metallic;

                float denom = 4.0f * max(dot(world_normal, view_dir), 0.0f) * max(dot(world_normal, l), 0.0f);
                vec3 specular = (ndf * G * F) / max(denom, 0.001f);

                float ndotl = max(dot(world_normal, l), 0.0f);
                lighting += (kd * albedo / (3.14159f + specular)) * radiance * ndotl;
            }
        }
    }

    vec3 ambient = 0.2f * albedo * Material.ambient.rgb;
    vec3 color = ambient + lighting;

    color = color / (color + vec3(1.0f));
    color = pow(color, vec3(1.0f / 2.20f));
    float fresnel = max(0.0f, 0.02f + (1.0f - 0.02f) * pow(1.0f - max(dot(view_dir,world_normal),0.0f),5.0f));
    fragColor = vec4(color, (1.0f - Material.alpha) * fresnel);
}
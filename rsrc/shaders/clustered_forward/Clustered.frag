#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (constant_id =  0) const uint ResolutionX = 1440;
layout (constant_id =  1) const uint ResolutionY = 900;
layout (constant_id =  2) const uint LightListMax = 512;
// TileSize vector in sample code
layout (constant_id =  3) const uint TileWidthX = 64;
layout (constant_id =  4) const uint TileWidthY = 64;
// Grid dimension vector in sampler code.
layout (constant_id =  5) const uint TileCountX = (ResolutionX - 1) / (TileWidthX + 1);
layout (constant_id =  6) const uint TileCountY = (ResolutionY - 1) / (TileWidthY + 1);
layout (constant_id =  7) const uint TileCountZ = 256;
layout (constant_id =  8) const float NearPlane = 0.1f;
layout (constant_id =  9) const float FarPlane = 3000.0f;
layout (constant_id = 10) const float AmbientGlobal = 0.20f;

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

layout (set = 2, binding = 0) uniform sampler2D diffuse;
layout (set = 2, binding = 1) uniform sampler2D bump;
layout (set = 2, binding = 2) uniform sampler2D roughness;
layout (set = 2, binding = 3) uniform sampler2D metallic;

uint CoordToIdx(uint i, uint j, uint k) {
    return TileCountX * TileCountY * k + TileCountX * j + i;
}

vec3 viewPosToGrid(vec2 frag_pos, float view_z) {
    vec3 c;
    c.xy = frag_pos / vec2(float(TileWidthX),float(TileWidthY));
    c.z = min(float(TileCountZ - 1), max(0.f, float(TileCountZ) * log((-view_z - NearPlane) / (FarPlane - NearPlane) + 1.0f)));
    return c;
}

void main() {
    vec3 diffuse_color = texture(diffuse_map, vUV).rgb * Material.diffuse.rgb;
    vec3 ambient = AmbientGlobal * Material.ambient.rgb;

    vec3 bitangent = cross(vTangent.xyz, vNormal.xyz);
    const mat3 inv_btn = inverse(transpose(mat3(vTangent,normalize(bitangent),vNormal)));
    vec3 normal_sample = texture(normal_map, vUV).rgb * vec3(2.0f) - vec3(1.0f);
    vec3 world_normal = inv_btn * normal_sample;

    vec3 view_dir = normalize(UBO.viewPosition.xyz - vPosition.xyz);
    if ((Material.diffuse.a < 1.0f) && (dot(view_dir, world_normal) < 0.0f)) {
        discard;
    }

    const float r0 = 0.02f;
    const float fresnel = max(0.0f, r0 + (1.0f - r0) * pow(1.0f - max(dot(view_dir, world_normal), 0.0f), 5.0f));
    vec3 specular = texture(specular_map, vUV).rgb * Material.specular.rgb;
    vec3 fresnel_specular = specular * (1.0f - specular) * fresnel;

    vec3 view_pos = (UBO.view * vPosition).xyz;
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
                vec3 l = normalize(light_pos_range.xyz - vPosition.xyz);
                vec3 h = normalize(0.5f * (view_dir + l));
                
                float lambertian = max(dot(world_normal, vec3(1.0f)), 0.0f);
                float atten = max(1.0f - max(0.0f, dist / light_pos_range.w), 0.0f);

                vec3 specular;
                if (Material.specular.a > 0.0f) {
                    vec3 bl_ph_spec = specular * pow(max(0.0f,dot(h,world_normal)),Material.specular.a);
                    specular = fresnel_specular + bl_ph_spec;
                }
                else {
                    specular = 0.50f * fresnel_specular;
                }

                vec3 light_color = imageLoad(lightColors, light_idx).rgb;
                lighting += light_color * lambertian * atten * (diffuse_color + specular);
            }
        }
    }

    fragColor = vec4(lighting + ambient, Material.diffuse.a + (1.0f - Material.diffuse.a) * fresnel);
}
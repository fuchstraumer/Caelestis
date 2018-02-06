#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (constant_index =  0) const uint TileCountZ = 256;
layout (constant_index =  1) const uint LightListMax = 512;
layout (constant_index =  2) const uint ResolutionX = 1440;
layout (constant_index =  3) const uint ResolutionY = 900;
// TileSize vector in sample code
layout (constant_index =  4) const uint TileWidthX = 64;
layout (constant_index =  5) const uint TileWidthY = 64;
layout (constant_index =  6) const float NearPlane = 0.1f;
layout (constant_index =  7) const float FarPlane = 3000.0f;
// Grid dimension vector in sampler code.
layout (constant_index =  8) const uint TileCountX = (ResolutionX - 1) / (TileWidthX + 1);
layout (constant_index =  9) const uint TileCountY = (ResolutionY - 1) / (TileWidthY + 1);
layout (constant_index = 10) const float AmbientGlobal = 0.20f;

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vNormal;
layout (location = 2) in vec4 vTangent;
layout (location = 3) in vec2 vUV;

layout (location = 0) out vec4 fragColor; 

// Global uniforms
layout (set = 2, binding = 0) uniform _ubo {
    mat4 model;
    mat4 view;
    mat4 projectionClip;
    mat4 normal;
    vec4 viewPosition;
    uint numLights;
} UBO;

// Clustered-forward technique uniforms
layout (set = 2, binding = 1, rgba32f) uniform readonly imageBuffer positionRanges;
layout (set = 2, binding = 2, rgba8) uniform readonly imageBuffer lightColors;
layout (set = 3, binding = 0, r8ui) uniform readonly uimageBuffer flags;
layout (set = 3, binding = 2, r32ui) uniform readonly uimageBuffer lightCounts;
layout (set = 3, binding = 4, r32ui) uniform readonly uimageBuffer lightCountOffsets;
layout (set = 3, binding = 5, r32ui) uniform readonly uimageBuffer lightList;

// Per-material uniforms
layout (set = 0, binding = 0) uniform _mtl {
    vec4 ambient;
    vec4 diffuse; // .a is alpha 
    vec4 specular; // .a is exponent
    vec4 emissive;
} Material;

layout (set = 1, binding = 0) uniform sampler2D diffuse;
layout (set = 1, binding = 1) uniform sampler2D opacity;
layout (set = 1, binding = 2) uniform sampler2D specular_map;
layout (set = 1, binding = 3) uniform sampler2D normal_map;

uint CoordToIdx(uint i, uint j, uint k) {
    return TileCountX * TileCountY * k + TileCountX * j + i;
}

vec3 viewPosToGrid(vec2 frag_pos, float view_z) {
    vec3 c(frag_pos / vec2(float(TileWidthX),float(TileWidthY)), 1.0f);
    c.z = min(float(TileCountZ - 1), max(0.f, float(TileCountZ) * log((-view_z - NearPlane) / (FarPlane - NearPlane) + 1.0f)));
    return c;
}

void main() {
    vec3 diffuse_color = texture(diffuse, vUV).rgb * Material.diffuse.rgb;
    vec3 ambient = AmbientGlobal * Material.ambient.rgb;

    vec3 bitangent = cross(vTangent, vNormal);
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
    uvec3 grid_coord = uvec3(viewPosToGrid(gl_FragCoord.xy), view_pos.z);
    int grid_idx = int(CoordToIdx(grid_coord.x, grid_coord.y, grid_coord.z));

    vec3 lighting = vec3(0.0f);

    if (imageLoad(flags, grid_idx).r == 1) {
        uint offset = imageLoad(lightCountOffsets, grid_idx).r;
        uint light_count = imageLoad(lightCounts, grid_idx).r;
        for (uint i = 0; i < light_count; ++i) {
            int light_idx = int(imageLoad(lightList, int(offset + i).r));
            vec4 light_pos_range = imageLoad(positionRanges, light_idx);
            float dist = distance(light_pos_range.xyz - vPosition.xyz);
            if (dist < light_pos_range.w) {
                vec3 l = normalize(light_pos_range.xyz - vPosition.xyz);
                vec3 h = normalize(0.5f * (view_dir + l));
                
                float lambertian = max(dot(world_normal, 1.0f), 0.0f);
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
                lighting += light_color * lambertian * atten * (diffuse + specular);
            }
        }
    }

    fragColor = vec4(lighting + ambient, Material.diffuse.a + (1.0f - Material.diffuse.a) * fresnel);
}
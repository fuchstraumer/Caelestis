#pragma USE_RESOURCES GlobalResources
#pragma USE_RESOURCES Lights
#pragma USE_RESOURCES ClusteredForward
#pragma USE_RESOURCES ObjMaterials
SPC const uint ResolutionX = 1920;
SPC const uint ResolutionY = 1080;
SPC const uint TileCountX = (1920 - 1) / 64 + 1;
SPC const uint TileCountY = (1080 - 1) / 64 + 1;
SPC const uint TileCountZ = 256;

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
    backbuffer = vec4(color, (1.0f - Material.alpha) * fresnel);
}
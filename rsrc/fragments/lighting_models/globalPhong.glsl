#define RESOURCES_BEGIN
uniform lights_data {
    vec4 lightPosition;
    vec4 lightColor;
} lightingData;
#define RESOURCES_END

#define MODEL_BEGIN
vec4 lightingModel(vec3 position, vec3 normal, vec3 tangent) {
    const float ambient_strength = 0.3f;
    vec3 ambient = ambient_strength * lightingData.lightColor.xyz;

    const float diffuse_strength = 0.6f;
    vec3 light_direction = normalize(lightingData.lightPosition.xyz - position);
    float diff = max(dot(normal, light_direction), 0.0f);
    vec3 diffuse = diffuse_strength * diff * lightingData.lightPosition.xyz;

    float specular_strength = 0.3f;
    vec3 view_direction = normalize(globals.viewPositions.xyz - position);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0f), 32.0f);
    vec3 specular = specular_strength * spec * lightingData.lightPosition.xyz;

    return vec4(ambient + diffuse + specular, 1.0f); 
}
#define MODEL_END
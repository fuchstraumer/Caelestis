void main() {
    vec3 fPos = subpassLoad(positionTexture).rgb;
    vec3 fNormal = subpassLoad(normalTexture).rgb;
    vec3 fTangent = subpassLoad(tangentTexture).rgb;
    backbuffer = subpassLoad(albedoTexture);

    backbuffer += lightingModel(fPos, fNormal, fTangent);
}
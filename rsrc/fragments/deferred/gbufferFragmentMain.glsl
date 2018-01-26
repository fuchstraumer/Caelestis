uniform sampler2D colorTexture;

layout (constant_id = 0) const float NEAR_PLANE = 0.01f;
layout (constant_id = 1) const float FAR_PLANE = 3000.0f;

float linearDepth(float depth) {
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

void main() {
    backbuffer = vec4(0.0); // required, but not actually performed iirc

    fAlbedo = texture(colorTexture, vUV);
    // storing depth in color alpha component
    fAlbedo.a = linearDepth(gl_FragCoord.z);

    fPosition = vec4(vPos, 1.0f);
    vec3 N = normalize(vNormal);
    N.y = -N.y;
    fNormal = vec4(N, 1.0f);
    fTangent = normalize(vTangent);
    fTangent.y = -fTangent.y;
}
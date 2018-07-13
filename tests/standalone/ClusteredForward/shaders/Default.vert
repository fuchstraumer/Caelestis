#pragma USE_RESOURCES GlobalResources
void main() {
    vPosition = mat3(UBO.model) * position;
    vNormal = mat3(UBO.normal) * normal;
    vTangent = mat3(UBO.normal) * tangent;
    vUV = uv;
    gl_Position = UBO.projectionClip * UBO.view * UBO.model * vec4(position, 1.0f);
}
#pragma USE_RESOURCES GlobalResources
void main() {
    vPosition = UBO.model * vec4(position, 1.0f);
    vNormal = UBO.normal * vec4(normal, 1.0f);
    vTangent = UBO.normal * vec4(tangent, 1.0f);
    vUV = uv;
    gl_Position = UBO.projectionClip * UBO.view * vPosition;
}
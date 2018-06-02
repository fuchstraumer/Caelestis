#pragma USE_RESOURCES GlobalResources
void main() {
    gl_Position = UBO.projectionClip * UBO.view * UBO.model * vec4(pos,1.0f);
}
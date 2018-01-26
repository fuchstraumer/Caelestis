void main() {
    gl_Position = matrices.projection * matrices.view * matrices.model * vec4(position, 1.0f);
    vPos = vec3(matrices.model * vec4(position, 1.0f));
    vPos.y *= -1.0f;
    mat3 normal_matrix = transpose(inverse(mat3(matrices.model)));
    vNormal = normal_matrix * normalize(normal);
    vTangent = normal_matrix * normalize(tangent);
}
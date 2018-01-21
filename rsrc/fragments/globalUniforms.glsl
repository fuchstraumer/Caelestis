
layout (binding = 0, set = 0) uniform matrix_ubo {
    mat4 model;
    mat4 view;
    mat4 projection;
} matrices;

layout (binding = 1, set = 0) uniform misc_data_ubo {
    vec4 viewPosition;
    vec2 mousePosition;
    vec2 windowSize;
    int frame;
} globals;
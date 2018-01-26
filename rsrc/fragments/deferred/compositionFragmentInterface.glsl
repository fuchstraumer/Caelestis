layout (input_attachment_index = 0, binding = $positions) uniform subpassInput positionTexture;
layout (input_attachment_index = 1, binding = $normals) uniform subpassInput normalTexture;
layout (input_attachment_index = 2, binding = $tangents) uniform subpassInput tangentTexture;
layout (input_attachment_index = 3, binding = $albedo) uniform subpassInput albedoTexture;

layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 backbuffer;
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec2 vUV;

layout (location = 0) out vec3 backbuffer; // albedo
layout (location = 1) out vec4 fPosition;
layout (location = 2) out vec4 fNormal;
layout (location = 3) out vec4 fTangent;
layout (location = 4) out vec4 fAlbedo;
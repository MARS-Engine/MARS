#version 450 core

layout(location = 0) in VS_OUT {
    vec3 pos;
    vec3 normal;
    vec2 uv;
} fs_in;

layout (binding = 3) uniform sampler2D tex;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMetal;
layout (location = 4) out vec4 outRough;
layout (location = 5) out vec4 outAO;

void main() {
    outAlbedo = texture(tex, fs_in.uv);
    if (outAlbedo.w == 0.0)
        discard;

    outPosition = vec4(fs_in.pos, 1.0f);
    outNormal = vec4(fs_in.normal, 1.0);
    outMetal = vec4(0, 0, 0, 1);
    outRough = vec4(0, 0, 0, 1);
    outAO = vec4(0, 0, 0, 1);
}

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

void main() {
    outPosition = vec4(fs_in.pos, 1.0f);
    outNormal = vec4(fs_in.normal, 1.0);
    outAlbedo = texture(tex, fs_in.uv);
    if (outAlbedo.w == 0.0)
        discard;
}

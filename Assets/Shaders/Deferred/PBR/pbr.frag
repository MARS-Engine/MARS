#version 450 core

layout(location = 0) in VS_OUT {
    vec3 pos;
    vec3 normal;
    vec2 uv;
} fs_in;

layout (binding = 1) uniform MATERIAL {
    vec4 diffuse;
} mat;

layout (binding = 2) uniform sampler2D albedo;
layout (binding = 3) uniform sampler2D normal;
layout (binding = 4) uniform sampler2D metal;
layout (binding = 5) uniform sampler2D rough;
layout (binding = 6) uniform sampler2D ao;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMetal;
layout (location = 4) out vec4 outRough;
layout (location = 5) out vec4 outAO;

vec3 process_normal()
{
    vec3 tangentNormal = texture(normal, fs_in.uv).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.pos);
    vec3 Q2  = dFdy(fs_in.pos);
    vec2 st1 = dFdx(fs_in.uv);
    vec2 st2 = dFdy(fs_in.uv);

    vec3 N   = normalize(fs_in.normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
    outPosition = vec4(fs_in.pos, 1.0f);
    outNormal = vec4(process_normal(), 1.0);
    outAlbedo = texture(albedo, fs_in.uv);
    outMetal = texture(metal, fs_in.uv);
    outRough = texture(rough, fs_in.uv);
    outAO = texture(ao, fs_in.uv);
}

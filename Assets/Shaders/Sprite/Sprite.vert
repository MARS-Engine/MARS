#version 450 core
layout (location = 0) in vec3 pos;

layout(location = 0) out VS_OUT {
    vec3 pos;
    vec3 normal;
    vec2 uv;
} vs_out;

layout (binding = 0) uniform MODEL {
	mat4 mvp;
} model;

layout (binding = 1) uniform UV {
    vec2 uv[4];
} uniUv;


void main() {
    gl_Position = model.mvp * vec4(pos, 1.0);
    vs_out.uv = uniUv.uv[gl_VertexIndex];
    vs_out.normal = vec3(0.0, 0.0, -1.0);
}
#version 450 core
layout (location = 0) in vec3 pos;

layout(location = 0) out VS_OUT {
    vec3 pos;
    vec3 normal;
    vec2 uv;
} vs_out;

layout (binding = 0) uniform MODEL {
	mat4 mvp;
    mat4 model;
} model;


layout (binding = 1) uniform UV {
    vec2 uv[4];
} uniUv;

layout (binding = 2) uniform SPRITE_RENDERER {
    bool flipX;
    bool flipY;
} renderer;


void main() {
    gl_Position = model.mvp * vec4(pos, 1.0);
    vs_out.pos = vec3(model.model * vec4(pos, 1.0));
    vs_out.uv = uniUv.uv[gl_VertexIndex];
    vs_out.uv = vec2(renderer.flipX ? 1.0 - vs_out.uv.x : vs_out.uv.x, renderer.flipY ? 1.0 - vs_out.uv.y : vs_out.uv.y);
    vs_out.normal = vec3(0.0, 0.0, -1.0);
}
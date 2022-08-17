#version 450 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout(location = 0) out VS_OUT {
    vec3 pos;
    vec3 normal;
    vec2 uv;
} vs_out;

layout (binding = 0) uniform MODEL {
    mat4 mvp;
    mat4 model;
} model;

void main() {
    gl_Position = model.mvp * vec4(pos, 1.0);
    vs_out.pos = pos;
    vs_out.normal = mat3(model.model) * normal;
    vs_out.uv = uv;
}
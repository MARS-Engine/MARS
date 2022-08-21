#version 450 core

layout(location = 0) in VS_OUT {
    vec3 pos;
    vec3 normal;
    vec2 uv;
} fs_in;

struct DirectionalLight {
    vec3 direction;
    vec4 color;
    vec4 ambient;
};

layout (binding = 1) uniform GLOBAL_LIGHT {
    DirectionalLight sun;
    vec3 test;
} gl;

layout (binding = 2) uniform MATERIAL {
    vec4 diffuse;
} mat;

layout (binding = 3) uniform sampler2D tex;

layout (location = 0) out vec4 FragColor;

vec4 CalculateDirectionalLight(DirectionalLight light, vec3 normal) {
    float diffuse_factor = max(dot(normal, -light.direction), 0.0);

    vec4 diff_samp = texture(tex, fs_in.uv);
    vec4 ambient = vec4(vec3(light.ambient * mat.diffuse), 0.0);
    vec4 diffuse = vec4(vec3(light.color * diffuse_factor), diff_samp.a);

    diffuse *= diff_samp;
    ambient *= diff_samp;

    return (ambient + diffuse);
}

void main() {
    FragColor = CalculateDirectionalLight(gl.sun, fs_in.normal);
}

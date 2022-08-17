#version 450 core

layout(location = 0) in VS_OUT {
    vec3 pos;
    vec3 normal;
    vec2 uv;
} fs_in;


layout (binding = 1) uniform GLOBAL_LIGHT {
    vec4 ambient;
} gl;

layout (binding = 2) uniform MATERIAL {
    vec4 diffuse;
} mat;

struct directionalLight {
    vec3 direction;
    vec4 color;
};

directionalLight sun  = {
    vec3(-0.57735, -0.57735, -0.57735),
    vec4(0.8, 0.8, 0.8, 1.0),
};

layout (binding = 3) uniform sampler2D tex;

layout (location = 0) out vec4 FragColor;

vec4 CalculateDirectionalLight(directionalLight light, vec3 normal) {
    float diffuse_factor = max(dot(normal, -light.direction), 0.0);

    vec4 diff_samp = texture(tex, fs_in.uv);
    vec4 ambient = vec4(vec3(gl.ambient * mat.diffuse), 0.0);
    vec4 diffuse = vec4(vec3(light.color * diffuse_factor), diff_samp.a);

    diffuse *= diff_samp;
    ambient *= diff_samp;

    return (ambient + diffuse);
}

void main() {
    FragColor = CalculateDirectionalLight(sun, fs_in.normal);
}

#version 450 core

layout(location = 0) in VS_OUT {
    vec3 pos;
    vec3 normal;
    vec2 uv;
} fs_in;

layout (binding = 2) uniform sampler2D tex;

layout (location = 0) out vec4 FragColor;

void main() {

    FragColor = texture(tex, fs_in.uv);
}

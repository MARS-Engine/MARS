#version 450

layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

struct DirectionalLight {
    vec3 direction;
    vec4 color;
    vec4 ambient;
};

layout (binding = 3) uniform GLOBAL_LIGHT {
    DirectionalLight sun;
} gl;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal) {
    vec3 pos = texture(samplerPosition, inUV).rgb;
    vec3 albedo = texture(samplerAlbedo, inUV).rgb;
    vec3 lighting = albedo * 0.1;
    vec3 viewDir = normalize(vec3(0.0, 0.0, -2.0) - pos);

    vec3 diffuse = max(dot(texture(samplerNormal, inUV).rgb, gl.sun.direction), 0.0) * albedo * gl.sun.color.xyz;
    return lighting + diffuse;
}

void main() {
    // Get G-Buffer values
    outFragcolor = vec4(CalculateDirectionalLight(gl.sun, texture(samplerNormal, inUV).rgb), 1.0);
}
#version 450

#define LIGHT_COUNT 1

layout (triangles, invocations = LIGHT_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 0) uniform UBO {
    mat4 mvp[LIGHT_COUNT];
} ubo;

layout (location = 0) in int inInstanceIndex[];

void main() {
    for (int i = 0; i < gl_in.length(); i++) {
        gl_Layer = gl_InvocationID;
        vec4 tmpPos = gl_in[i].gl_Position;
        gl_Position = ubo.mvp[gl_InvocationID] * tmpPos;
        EmitVertex();
    }
    EndPrimitive();
}

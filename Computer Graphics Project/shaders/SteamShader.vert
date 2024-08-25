#version 450

layout(binding = 0) uniform SteamUniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    float time;
} subo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNorm;

layout(location = 0) out vec2 fragUV;

void main() {
    float time = subo.time;
    vec3 pos = inPos;
    pos.x += sin(time + inPos.y * 2.0) * 0.1;       // Wiggle effect on the x-axis
    pos.y += sin(time + inPos.x * 2.0) * 0.1;       // Wiggle effect on the y-axis

    gl_Position = subo.mvpMat * vec4(pos, 1.0);
    fragUV = inUV;
}
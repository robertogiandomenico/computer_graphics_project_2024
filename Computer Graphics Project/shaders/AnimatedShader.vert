#version 450

layout(binding = 0) uniform AnimatedUniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    float time;
    float speed;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNorm;

layout(location = 0) out vec2 fragUV;

void main() {
    float time = ubo.time;
    float speed = ubo.speed;
    vec3 pos = inPos;

    pos.x += sin(time * speed + inPos.y * 2.0) * 0.1;       // Wiggle effect on the x-axis
    pos.y += sin(time * speed + inPos.x * 2.0) * 0.1;       // Wiggle effect on the y-axis

    gl_Position = ubo.mvpMat * vec4(pos, 1.0);
    fragUV = inUV;
}
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform AnimatedUniformBufferObject {
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
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec3 fragPos;

void main() {
    fragUV = inUV;
    fragNorm = mat3(ubo.nMat) * inNorm;  // Transforming normal with the normal matrix
    fragPos = vec3(ubo.mMat * vec4(inPos, 1.0)); // Position in world space
    float time = ubo.time;
    float speed = ubo.speed;
    vec3 pos = inPos;

    pos.y += 0.05 * sin(time * 2);                              // Rigid translation on the y-axis (vertically)
    pos.z -= sin(time * speed + inPos.x * 3.0) * 0.02;          // Wiggle effect on the z-axis

    gl_Position = ubo.mvpMat * vec4(pos, 1.0);
}
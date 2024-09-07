#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform UniformBufferObject {
    mat4 mvpMat;  // Model-View-Projection matrix
    mat4 mMat;    // Model matrix
    mat4 nMat;    // Normal matrix (transpose of the inverse of the model matrix)
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNorm;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec3 fragPos;

void main() {
    vec3 pos = inPos;

    fragUV = inUV;
    fragNorm = mat3(ubo.nMat) * inNorm;  // Transforming normal with the normal matrix
    fragPos = vec3(ubo.mMat * vec4(inPos, 1.0)); // Position in world space

    gl_Position = ubo.mvpMat * vec4(inPos, 1.0);
}
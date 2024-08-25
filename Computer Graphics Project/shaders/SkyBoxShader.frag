#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 mvpMat;
    float time;  // Include time in the same uniform block
} ubo;

layout(location = 0) in vec3 fragTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D skybox;

// Function to rotate the hue of a color
vec3 hueRotate(vec3 color, float angle) {
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);

    mat3 rotationMatrix = mat3(
        0.299, 0.587, 0.114,
        0.299, 0.587, 0.114,
        0.299, 0.587, 0.114
    ) + mat3(
        0.701, -0.587, -0.114,
        -0.299, 0.413, -0.114,
        -0.3, -0.588, 0.886
    ) * cosAngle + mat3(
        0.168, 0.330, -0.497,
        -0.328, 0.035, 0.292,
        1.25, -1.05, -0.203
    ) * sinAngle;

    return color * rotationMatrix;
}

void main() {
    float yaw = -(atan(fragTexCoord.x, fragTexCoord.z) / 6.2831853 + 0.5);
    float pitch = -(atan(fragTexCoord.y, sqrt(fragTexCoord.x * fragTexCoord.x + fragTexCoord.z * fragTexCoord.z)) / 3.14159265 + 0.5);
    
    // Sample the color from the texture
    vec4 sampledColor = texture(skybox, vec2(yaw, pitch));
    
    // Rotate the hue of the sampled color based on the time uniform
    vec3 rotatedColor = hueRotate(sampledColor.rgb, ubo.time);
    
    outColor = vec4(rotatedColor, sampledColor.a);
}

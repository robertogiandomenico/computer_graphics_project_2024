#version 450

layout(binding = 2) uniform GlobalUniformBufferObject {
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
} gubo;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    vec3 norm = normalize(fragNorm);
    vec3 lightDir = normalize(gubo.lightDir);

    // Ambient
    vec3 ambient = 0.1 * vec3(gubo.lightColor);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(gubo.lightColor);

    // Specular
    vec3 viewDir = normalize(gubo.eyePos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * vec3(gubo.lightColor);

    // Combine results
    vec3 result = ambient + diffuse + specular;
    vec4 texColor = texture(texSampler, fragUV);
    outColor = vec4(result * texColor.rgb, texColor.a);
}
#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHTS_NUM 6

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
    vec3 lightPos[LIGHTS_NUM];          // Position of the lights
    vec3 lightColor[LIGHTS_NUM];        // Color of the lights
    vec3 eyePos;                        // Position of the camera/eye
    float constant[LIGHTS_NUM];         // Constant attenuation factor
    float linear[LIGHTS_NUM];           // Linear attenuation factor
    float quadratic[LIGHTS_NUM];        // Quadratic attenuation factor
} gubo;

// New uniform for emissive color
layout(binding = 3) uniform EmissiveUniformBufferObject {
    vec3 emissiveColor;  // Emissive color of the object
} eubo;

void main() {
    vec3 norm = normalize(fragNorm);
    vec3 viewDir = normalize(gubo.eyePos - fragPos);
    vec3 result = vec3(0.0); // Initialize result color

    for (int i = 0; i < LIGHTS_NUM; ++i) {
        vec3 fragToLight = gubo.lightPos[i] - fragPos;
        vec3 lightDir = normalize(fragToLight);

        // Ambient
        vec3 ambient = 0.1 * vec3(gubo.lightColor[i]);

        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(gubo.lightColor[i]);

        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = spec * vec3(gubo.lightColor[i]);

        // Attenuation
        float distance = length(fragToLight);

        float constantFactor = max(gubo.constant[i], 0.01); // Ensure it's not zero or too small
        float linearFactor = max(gubo.linear[i], 0.01);
        float quadraticFactor = max(gubo.quadratic[i], 0.01);

        float epsilon = 0.001;
        float attenuation = 1.0 / (constantFactor + linearFactor * distance + quadraticFactor * (distance * distance));


        // Accumulate the result from this light
        result += (ambient + diffuse + specular) * attenuation;
    }

    vec4 texColor = texture(texSampler, fragUV);

    // Add emissive color to the final output
    vec3 emissive = eubo.emissiveColor * texColor.rgb;
    result += emissive;

    outColor = vec4(result * texColor.rgb, texColor.a);
}

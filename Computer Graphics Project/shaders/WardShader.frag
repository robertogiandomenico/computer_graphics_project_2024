#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHTS_NUM 16
#define COLLECTIBLES_NUM 7

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec4 fragTan;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D tex;

layout(binding = 2) uniform GlobalUniformBufferObject {
    vec3 lightDir[LIGHTS_NUM];      // Direction of the lights
    vec3 lightPos[LIGHTS_NUM];      // Position of the lights
    vec4 lightColor[LIGHTS_NUM];    // Color of the lights
    vec3 eyePos;					// Position of the camera/eye
    vec4 lightOn;					// Lights on/off flags (point, direct, spot, ambient component)
    float cosIn;					// Spot light inner cone angle
	float cosOut;					// Spot light outer cone angle
} gubo;

// New uniform for emissive color
layout(binding = 3) uniform EmissiveUniformBufferObject {
    vec3 emissiveColor;  // Emissive color of the object
} eubo;

layout(binding = 4) uniform sampler2D spet;

layout(binding = 5) uniform sampler2D norm;

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 T, vec3 B, vec3 Md, vec3 Ms, float alphaT, float alphaB) {
/* This BRDF should perform the Ward anisotropic specular model with the Lambert diffuse model.
		 
Paramters:
	V	   - Viewer direction
	N	   - Normal vector direction
	L	   - Light direction
	T	   - Tangent vector direction
	B	   - Bitangent vector direction
	Md	   - Diffuse color
	Ms	   - Specular color
	alphaT - Roughness along the tangent direction
	alphaB - Roughness along the bitangent direction
*/ 
	vec3 Diffuse = Md * clamp(dot(N, L),0.0,1.0);

    // ward anisotropic specular
    vec3 H = normalize(V + L);
    float h_t = dot(H, T);
    float h_b = dot(H, B);
    float h_n = dot(H, N);
    float l_n = dot(L, N);
    float v_n = dot(V, N);

    float epsilon = 1e-6;

    float exponent = (pow(h_t / alphaT, 2) + pow(h_b / alphaB, 2)) / pow(h_n, 2);
    float denominator = 4.0 * alphaT * alphaB * sqrt(h_n / (l_n * v_n)) * radians(180.0f);
    denominator = max(denominator, epsilon);

    vec3 Specular = Ms * exp(-exponent) / denominator;
    Specular = max(Specular, epsilon);

	return (Diffuse + Specular);
}

void main() {

	// Sample the normal map
    vec3 normalMapSample = texture(norm, fragUV).rgb;

	// Convert from [0,1] range to [-1,1] range
    vec3 normalMapNormal = normalize(normalMapSample * 2.0 - 1.0);

	vec3 Norm = normalize(fragNorm);
	vec3 Tan = normalize(fragTan.xyz - Norm * dot(fragTan.xyz, Norm));
	vec3 Bitan = cross(Norm, Tan) * fragTan.w;
	vec3 N = normalize(normalMapNormal * Tan + normalMapNormal * Bitan + normalMapNormal * fragNorm);

	vec3 albedo  = texture(tex,  fragUV).rgb;
	vec3 specCol = texture(spet, fragUV).rgb;

	vec3 finalColor = vec3(0.0f);

	vec3 V = normalize(gubo.eyePos - fragPos);

	for (int i = 0; i < (LIGHTS_NUM - COLLECTIBLES_NUM - 1); ++i) {
        // contribution of all lights except spot lights
        
        vec3 L;
        float attenuation = 1.0;
        
        if (length(gubo.lightDir[i]) > 0.0) {
            // directional light
            L = normalize(gubo.lightDir[i]);
            attenuation = 1.0 * gubo.lightOn.y;
        } else {
            // point lights
            L = normalize(gubo.lightPos[i] - fragPos);
            
            float distance = length(gubo.lightPos[i] - fragPos);
            attenuation = 1.0 / (0.1 + 0.1 * distance + 0.01 * distance * distance) * gubo.lightOn.x;   // standard attenuation formula
        }

        vec3 DiffSpec = BRDF(V, N, L, Tan, Bitan, albedo, specCol, 0.1f, 0.1f);
        
        finalColor += DiffSpec * gubo.lightColor[i].rgb * attenuation;
    }

    // Add emissive color
	finalColor += eubo.emissiveColor * albedo;
	
    // Add ambient light
	vec3 ambient = 0.3 * albedo;
    finalColor += ambient * gubo.lightOn.w;
	
	outColor = vec4(finalColor, 1.0f);
}
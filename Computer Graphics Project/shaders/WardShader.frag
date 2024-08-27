#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec4 fragTan;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D tex;

layout(binding = 2) uniform GlobalUniformBufferObject {
    vec3 lightDir[9];          // Direction of the lights
    vec3 lightPos[9];          // Position of the lights
    vec4 lightColor[9];        // Color of the lights
    vec3 eyePos;               // Position of the camera/eye
    vec4 lightOn;              // Lights on/off flags (point, direct, spot, ambient component)
    float cosIn;               // Spot light inner cone angle
	float cosOut;              // Spot light outer cone angle
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

    float exponent = (pow(h_t / alphaT, 2) + pow(h_b / alphaB, 2)) / pow(h_n, 2);
    float denominator = 4.0 * alphaT * alphaB * sqrt(h_n / (l_n * v_n)) * radians(180.0f);

    vec3 Specular = Ms * exp(-exponent) / denominator;

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

	for (int i = 0; i < 9; ++i) {
        vec3 L;
        
        if (length(gubo.lightDir[i]) > 0.0) {
            // Directional light
            L = normalize(gubo.lightDir[i]);
        } else {
            // Point or spot light
            L = normalize(gubo.lightPos[i] - fragPos);
        }

        float distance = length(gubo.lightPos[i] - fragPos);
        float attenuation = 1.0 / (0.1 + 0.1 * distance + 0.01 * distance * distance); // Standard attenuation formula
		// float attenuation = 1.0 / (distance * distance); // Inverse square attenuation formula

        vec3 DiffSpec = BRDF(V, N, L, Tan, Bitan, albedo, specCol, 0.1f, 0.4f);
        
        finalColor += DiffSpec * gubo.lightColor[i].rgb * attenuation;
    }

	finalColor += eubo.emissiveColor;

	/*
	const vec3 cxp = vec3(1.0,0.5,0.5) * 0.15;
	const vec3 cxn = vec3(0.9,0.6,0.4) * 0.15;
	const vec3 cyp = vec3(0.3,1.0,1.0) * 0.15;
	const vec3 cyn = vec3(0.5,0.5,0.5) * 0.15;
	const vec3 czp = vec3(0.8,0.2,0.4) * 0.15;
	const vec3 czn = vec3(0.3,0.6,0.7) * 0.15;
	
	vec3 Ambient =((N.x > 0 ? cxp : cxn) * (N.x * N.x) +
				   (N.y > 0 ? cyp : cyn) * (N.y * N.y) +
				   (N.z > 0 ? czp : czn) * (N.z * N.z)) * albedo;
	*/
	outColor = vec4(finalColor, 1.0f);
}
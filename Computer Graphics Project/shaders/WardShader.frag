#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHTS_NUM 16
#define COLLECTIBLES_NUM 7

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec4 fragTan;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    vec3 lightDir[LIGHTS_NUM];      // Direction of the lights
    vec3 lightPos[LIGHTS_NUM];      // Position of the lights
    vec4 lightColor[LIGHTS_NUM];    // Color of the lights
    vec3 eyePos;					// Position of the camera/eye
    vec4 lightOn;					// Lights on/off flags (point, direct, spot, ambient component)
    float cosIn;					// Spot light inner cone angle
	float cosOut;					// Spot light outer cone angle
    bool gameOver;                  // Game over flag
} gubo;

layout(set = 1, binding = 1) uniform sampler2D tex;

// Uniform for emissive color
layout(set = 1, binding = 2) uniform EmissiveUniformBufferObject {
    vec3 emissiveColor;  // Emissive color of the object
} eubo;

layout(set = 1, binding = 3) uniform sampler2D spet;

layout(set = 1, binding = 4) uniform sampler2D norm;

// Direct light
vec3 direct_light_dir(vec3 fragPos, int i) {
    return normalize(gubo.lightDir[i]);
}

vec3 direct_light_color(vec3 fragPos, int i) {
	return gubo.lightColor[i].rgb;
}

// Point light
vec3 point_light_dir(vec3 fragPos, int i) {
    vec3 p = gubo.lightPos[i];
	return normalize(p - fragPos);
}

vec3 point_light_color(vec3 fragPos, int i) {
    vec3 l = gubo.lightColor[i].rgb;
    float g = gubo.lightColor[i].a; // scaling factor
    float beta = 2.0f;  // inverse-squared decay factor
    vec3 p = gubo.lightPos[i];

    return pow(g / length(p - fragPos), beta) * l;
}

// Spot light
vec3 spot_light_dir(vec3 fragPos, int i) {
	vec3 p = gubo.lightPos[i];
	return normalize(p - fragPos);
}

vec3 spot_light_color(vec3 fragPos, int i) {
    vec3 l = gubo.lightColor[i].rgb;
    float g = gubo.lightColor[i].a; // scaling factor
    float beta = 1.0f;  // inverse-linear decay factor
    vec3 p = gubo.lightPos[i];
    vec3 d = gubo.lightDir[i];
    float cosin;
    float cosout;

    if (i == 8) {
        cosin = gubo.cosIn;
        cosout = gubo.cosOut;
    } else {
        cosin = gubo.cosIn + radians(10.0f);
        cosout = gubo.cosOut + radians(10.0f);
    }

    return pow(g/length(p - fragPos), beta) * clamp((dot(normalize(p - fragPos), d) - cosout)/(cosin - cosout), 0.0, 1.0) * l;
}

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 T, vec3 B, vec3 Md, vec3 Ms, float alphaT, float alphaB) {
/* BRDF function: Ward anisotropic specular model with Lambert diffuse model
		 
Parameters:
	V	   - Viewer direction
	N	   - Normal vector direction
	L	   - Light direction
	T	   - Tangent vector direction
	B	   - Bitangent vector direction
	Md	   - Diffuse color
	Ms	   - Specular color
	alphaT - Roughness along tangent direction
	alphaB - Roughness along bitangent direction
*/ 

	// Lambert
	vec3 Diffuse = Md * clamp(dot(N, L),0.0,1.0);

    // Ward
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

	return Diffuse + Specular;
}

void main() {
	// Sample the normal map
    vec3 normalMapSample = texture(norm, fragUV).rgb;

	vec3 Norm = normalize(fragNorm);
	vec3 Tan = normalize(fragTan.xyz - Norm * dot(fragTan.xyz, Norm));
	vec3 Bitan = cross(Norm, Tan) * fragTan.w;
    mat3 tbn = mat3(Tan, Bitan, Norm);
	vec3 N = normalize(tbn * (normalMapSample * 2.0 - 1.0));

    // Sample textures
	vec3 albedo  = texture(tex,  fragUV).rgb;
	vec3 specCol = texture(spet, fragUV).rgb;

    // Metallic Factor
    float metallic = 1.0;

    // Adjust Diffuse and Specular Based on Metallic
    // For non-metallic (metallic = 0.0): Md = albedo, Ms = vec3(0.04)
    // For metallic (metallic = 1.0): Md = vec3(0.0), Ms = specCol
    vec3 Md = mix(albedo * (1.0 - metallic), vec3(0.0), metallic);
    vec3 Ms = mix(vec3(0.04), specCol, metallic);

    vec3 V = normalize(gubo.eyePos - fragPos);

	vec3 result = vec3(0.0f);   // Initialize result color
    vec3 ambient = vec3(0.0f);  // Initialize ambient color

    vec3 LD;
    vec3 LC;

    // Add the point lights
    for (int i = 0; i < (LIGHTS_NUM - COLLECTIBLES_NUM - 2); ++i) {   
        LD = point_light_dir(fragPos, i);
        LC = point_light_color(fragPos, i);

        result += BRDF(V, N, LD, Tan, Bitan, Md, Ms, 0.1f, 0.1f) * LC * gubo.lightOn.x;
    }
    
    // Add the directional light
    LD = direct_light_dir(fragPos, 7);
    LC = direct_light_color(fragPos, 7);

    result += BRDF(V, N, LD, Tan, Bitan, Md, Ms, 0.1f, 0.1f) * LC * gubo.lightOn.y;
    
    // Add the cauldron spot light if the game is over
    if (gubo.gameOver) {
        LD = spot_light_dir(fragPos, 8);
        LC = spot_light_color(fragPos, 8);

        result += BRDF(V, N, LD, Tan, Bitan, Md, Ms, 0.1f, 0.1f) * LC * gubo.lightOn.z;
    }

    // Add the collectibles spot lights
    for (int i=0; i < COLLECTIBLES_NUM; i++) {
        LD = spot_light_dir(fragPos, i+(LIGHTS_NUM-COLLECTIBLES_NUM) );
        LC = spot_light_color(fragPos, i+(LIGHTS_NUM-COLLECTIBLES_NUM) );

        result += BRDF(V, N, LD, Tan, Bitan, Md, Ms, 0.1f, 0.1f) * LC * gubo.lightOn.z;
    }

    // Add emissive color
	result += eubo.emissiveColor * albedo * albedo;
	
    // Add ambient light
	ambient = 0.3 * albedo;
    result += ambient * gubo.lightOn.w;
	
	outColor = vec4(result, 1.0f);
}
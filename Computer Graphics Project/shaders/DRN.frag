#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHTS_NUM 16
#define COLLECTIBLES_NUM 7

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec4 fragTan;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;       // Diffuse map

layout(set = 1, binding = 0) uniform GlobalUniformBufferObject {
    vec3 lightDir[LIGHTS_NUM];                  // Direction of the lights
    vec3 lightPos[LIGHTS_NUM];                  // Position of the lights
    vec4 lightColor[LIGHTS_NUM];                // Color of the lights
    vec3 eyePos;                                // Position of the camera/eye
    vec4 lightOn;                               // Lights on/off flags (point, direct, spot, ambient component)
    float cosIn;                                // Spot light inner cone angle
	float cosOut;                               // Spot light outer cone angle
    bool gameOver;                              // Game over flag
} gubo;

// Uniform for emissive color
layout(binding = 2) uniform EmissiveUniformBufferObject {
    vec3 emissiveColor;  // Emissive color of the object
} eubo;

layout(binding = 3) uniform sampler2D normalMap;        // Normal map

layout(binding = 4) uniform sampler2D roughnessMap;     // Roughness map

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

// BRDF function
vec3 BRDF(vec3 Albedo, vec3 Norm, vec3 EyeDir, vec3 LightDir, float Roughness) {
    // Compute diffuse component
    vec3 Diffuse = Albedo * max(dot(Norm, LightDir), 0.0f);

    // Compute specular component
    vec3 halfwayDir = normalize(LightDir + EyeDir);
    float n_h = dot(Norm, halfwayDir);
    float specPower = mix(32.0f, 128.0f, 1.0 - Roughness); // Dynamic specular power based on roughness
    vec3 Specular = vec3(pow(max(n_h, 0.0f), specPower));

    return Diffuse + Specular;
}

void main() {
    // Sample the textures
    vec4 texColor = texture(texSampler, fragUV);
    vec3 Albedo = texColor.rgb;
    float alpha = texColor.a;
    float Roughness = texture(roughnessMap, fragUV).r;

    // Sample the normal map
    vec3 normalMapSample = texture(normalMap, fragUV).rgb;

	vec3 N = normalize(fragNorm);
	vec3 Tan = normalize(fragTan.xyz - N * dot(fragTan.xyz, N));
	vec3 Bitan = cross(N, Tan) * fragTan.w;
    mat3 tbn = mat3(Tan, Bitan, N);
	vec3 Norm = normalize(tbn * (normalMapSample * 2.0 - 1.0));

    vec3 EyeDir = normalize(gubo.eyePos - fragPos);
    
    vec3 LD;
    vec3 LC;

    vec3 result = vec3(0.0); // Initialize result color
    vec3 ambient = vec3(0.0); // Initialize ambient color

    // Add the point lights
    for (int i = 0; i < (LIGHTS_NUM - COLLECTIBLES_NUM - 2); ++i) {       
        LD = point_light_dir(fragPos, i);
        LC = point_light_color(fragPos, i);

        result += BRDF(Albedo, Norm, EyeDir, LD, Roughness) * LC * gubo.lightOn.x;
    }
    
    // Add the directional light
    LD = direct_light_dir(fragPos, 7);
    LC = direct_light_color(fragPos, 7);

    result += BRDF(Albedo, Norm, EyeDir, LD, Roughness) * LC * gubo.lightOn.y;

    // Add the cauldron spot light if the game is over
    if (gubo.gameOver) {
        LD = spot_light_dir(fragPos, 8);
        LC = spot_light_color(fragPos, 8);

        result += BRDF(Albedo, Norm, EyeDir, LD, Roughness) * LC * gubo.lightOn.z;
    }

    // Add the collectibles spot lights
    for (int i=0; i < COLLECTIBLES_NUM; i++) {
        LD = spot_light_dir(fragPos, i+(LIGHTS_NUM-COLLECTIBLES_NUM) );
        LC = spot_light_color(fragPos, i+(LIGHTS_NUM-COLLECTIBLES_NUM) );

        result += BRDF(Albedo, Norm, EyeDir, LD, Roughness) * LC * gubo.lightOn.z;
    }

    // Add emissive color
    vec3 emissive = eubo.emissiveColor * Albedo * Albedo;
    result += emissive;
    
    // Add ambient light
    ambient = 0.05 * Albedo;
    result += ambient * gubo.lightOn.w;

    outColor = vec4(result, alpha);
}
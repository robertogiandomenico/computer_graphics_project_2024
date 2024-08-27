#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHTS_NUM 9

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
    vec3 lightDir[LIGHTS_NUM];          // Direction of the lights
    vec3 lightPos[LIGHTS_NUM];          // Position of the lights
    vec4 lightColor[LIGHTS_NUM];        // Color of the lights
    vec3 eyePos;                        // Position of the camera/eye
    vec4 lightOn;                       // Lights on/off flags (point, direct, spot, ambient component)
    float cosIn;                        // Spot light inner cone angle
	float cosOut;                       // Spot light outer cone angle
} gubo;

// New uniform for emissive color
layout(binding = 3) uniform EmissiveUniformBufferObject {
    vec3 emissiveColor;  // Emissive color of the object
} eubo;

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
    float cosin = gubo.cosIn;
    float cosout = gubo.cosOut;

    return pow(g/length(p - fragPos), beta) * clamp((dot(normalize(p - fragPos), d) - cosout)/(cosin - cosout), 0.0, 1.0) * l;
}

vec3 BRDF(vec3 Albedo, vec3 Norm, vec3 EyeDir, vec3 LD) {
// Compute the BRDF, with a given color <Albedo>, in a given position characterized by a given normal vector <Norm>,
// for a light direct according to <LD>, and viewed from a direction <EyeDir>
	vec3 Diffuse = Albedo * max(dot(Norm, LD), 0.0f);
	vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(LD, Norm)), 0.0f), 32));
	
	return Diffuse + Specular;
}

void main() {
    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos - fragPos);
    
    vec4 texColor = texture(texSampler, fragUV);
    vec3 Albedo = texColor.rgb;

    vec3 LD;
    vec3 LC;

    vec3 result = vec3(0.0); // Initialize result color
    vec3 ambient = vec3(0.0); // Initialize ambient color

    for (int i = 0; i < LIGHTS_NUM - 2; ++i) {       
        LD = point_light_dir(fragPos, i);
        LC = point_light_color(fragPos, i);

        result += BRDF(Albedo, Norm, EyeDir, LD) * LC * gubo.lightOn.x;
    
        // Ambient
        ambient = 0.003 * vec3(gubo.lightColor[i].rgb);

        // Accumulate the result from this light
        result += ambient * gubo.lightOn.w;
    }
    
    // Add the directional light
    LD = direct_light_dir(fragPos, 7);
    LC = direct_light_color(fragPos, 7);

    result += BRDF(Albedo, Norm, EyeDir, LD) * LC * gubo.lightOn.y;

    ambient = 0.003 * vec3(gubo.lightColor[7].rgb);
    result += ambient * gubo.lightOn.w;

    // Add the spot light
    LD = spot_light_dir(fragPos, 8);
    LC = spot_light_color(fragPos, 8);

    result += BRDF(Albedo, Norm, EyeDir, LD) * LC * gubo.lightOn.z;

    ambient = 0.003 * vec3(gubo.lightColor[8].rgb);
    result += ambient * gubo.lightOn.z;

    // Add emissive color to the final output
    vec3 emissive = eubo.emissiveColor * texColor.rgb;
    result += emissive;

    outColor = vec4(result, 1.0f);
}

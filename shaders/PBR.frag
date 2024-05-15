#version 460

#define PI 3.14159265

// Uniforms
layout(location = 6) uniform vec3 lightPos;
layout(location = 7) uniform vec3 lightColor;

layout(location = 8) uniform vec3 albedo;
layout(location = 9) uniform float metallic;
layout(location = 10) uniform float roughness;
layout(location = 11) uniform float ao; // Ambient occlusion
layout(location = 12) uniform float foamStrength;
layout(location = 13) uniform int wireframe;

// Samplers
layout(binding = 3) uniform sampler2D derivatives1;
layout(binding = 4) uniform sampler2D derivatives2;
layout(binding = 5) uniform sampler2D derivatives3;
layout(binding = 6) uniform sampler2D turbulence1;
layout(binding = 7) uniform sampler2D turbulence2;
layout(binding = 8) uniform sampler2D turbulence3;

// Passthroughs
in vec3 outPos;
flat in int outSize;
flat in int outScale1;
flat in int outScale2;
flat in int outScale3;
in vec3 outViewPos;
in vec3 outLods;

out vec4 finalColor;

// Lighting model from https://learnopengl.com/PBR/Lighting
float DistributionGGX(vec3 normal, vec3 halfwayVector, float roughness) {
	float a = roughness;
	float a2 = a * a;
	float NdotH = max(dot(normal, halfwayVector), 0.0);
	float NdotH2 = NdotH * NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 normal, vec3 viewDir, vec3 lightVector, float roughness) {
	float NdotV = max(dot(normal, viewDir), 0.0);
	float NdotL = max(dot(normal, lightVector), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 LightingEquation(vec3 mAlbedo, float jacobian, vec3 normal, vec3 viewDir, vec3 lightPos) {
	// The base reflectance of water is 0.02
	vec3 F0 = vec3(0.02);
	F0 = mix(F0, mAlbedo, metallic);

	// Reflectance equation
	vec3 Lo = vec3(0.0);
	
	vec3 lightVector = normalize(lightPos - outPos);
	vec3 halfwayVector = normalize(viewDir + lightVector);

	// Distance from vertex to light
	float dist = length(lightPos - outPos);
	// Normal inverse square law attenuation has too much of a falloff to simulate a far sun, so we use a linear falloff
	float attenuation = 1 / dist;
	vec3 radiance = lightColor * attenuation;

	// Cook-Torrence BRDF
	float NDF = DistributionGGX(normal, halfwayVector, roughness);
	float G = GeometrySmith(normal, viewDir, lightVector, roughness);
	vec3 F = FresnelSchlick(clamp(dot(halfwayVector, viewDir), 0.0, 1.0), F0);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightVector), 0.0) + 0.0001; // 0.0001 prevents division by 0
	vec3 specular = numerator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	float NdotL = max(dot(normal, lightVector), 0.0);

	Lo += (kD * mAlbedo / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.03) * mAlbedo * ao;
	vec3 color = ambient + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// Gamma correction
	color = pow(color, vec3(1.0/2.2));

	return color;
}

void main() {
	vec2 coords = outPos.xz;

	vec4 derivatives = texture(derivatives1, coords / outScale1) * outLods.x;
	derivatives		+= texture(derivatives2, coords / outScale2) * outLods.y;
	derivatives		+= texture(derivatives3, coords / outScale3) * outLods.z;
	
	vec2 slopeVector = vec2(derivatives.x / (1 + derivatives.z), derivatives.y / (1 + derivatives.w));
	vec3 normal = normalize(vec3(-slopeVector.x, 1, -slopeVector.y));
	vec3 viewDir = normalize(outViewPos - outPos);

	float jacobian = texture(turbulence1, coords / outScale1).x;
	jacobian	  += texture(turbulence2, coords / outScale2).x;
	jacobian	  += texture(turbulence3, coords / outScale3).x;
	jacobian = min(1, max(0, foamStrength - jacobian));
	
	float fresnel = dot(normal, viewDir);
	fresnel = clamp(1 - fresnel, 0.0, 1.0);
	fresnel = pow(fresnel, 5);
	
	vec3 emission = mix(vec3(0), vec3(255) * (1 - fresnel), jacobian);
	vec3 color = LightingEquation(albedo, jacobian, normal, viewDir, lightPos);

	if (wireframe == 1) {
		finalColor = vec4(0);
	} else {
		finalColor = vec4(color + emission, 1.0);
	}
}
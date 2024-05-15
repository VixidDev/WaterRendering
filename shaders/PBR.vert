#version 460

// VAO attributes
layout(location = 0) in vec3 iPosition;

// Uniforms
layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform int size;
layout(location = 2) uniform int scale1;
layout(location = 3) uniform int scale2;
layout(location = 4) uniform int scale3;
layout(location = 5) uniform vec3 camPos;

// Samplers
layout(binding = 0) uniform sampler2D displacements1;
layout(binding = 1) uniform sampler2D displacements2;
layout(binding = 2) uniform sampler2D displacements3;

// Fragment passthroughs
out vec3 outPos;
out int outSize;
out int outScale1;
out int outScale2;
out int outScale3;
out vec3 outViewPos;
out vec3 outLods;

void main() {
	outPos = iPosition;
	outSize = size;
	outScale1 = scale1;
	outScale2 = scale2;
	outScale3 = scale3;
	outViewPos = camPos;

	float viewDist = length(camPos - iPosition);
	float lod1 = min(10 * scale1 / viewDist, 1);
	float lod2 = min(10 * scale2 / viewDist, 1);
	float lod3 = min(10 * scale3 / viewDist, 1);
	outLods = vec3(lod1, lod2, lod3);

	vec2 coords = iPosition.xz;

	vec3 displacements = vec3(0);
	displacements += texture(displacements1, coords / scale1).xyz;
	displacements += texture(displacements2, coords / scale2).xyz;
	displacements += texture(displacements3, coords / scale3).xyz;

	vec4 finalPos = mvpMatrix * vec4(iPosition + displacements, 1.0);

	gl_Position = finalPos;
}
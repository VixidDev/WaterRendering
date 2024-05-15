#version 460

// Samplers
layout(binding = 0) uniform samplerCube skybox;

// Fragment passthroughs
in vec3 outTexCoords;

out vec4 outColor;

void main() {
	outColor = texture(skybox, outTexCoords);
}
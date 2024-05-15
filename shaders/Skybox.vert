#version 460

// VAO attributes
layout(location = 0) in vec3 iPosition;

// Uniforms
layout(location = 0) uniform mat4 mvpMatrix;

// Fragment passthroughs
out vec3 outTexCoords;

void main() {
	outTexCoords = iPosition;
	gl_Position = (mvpMatrix * vec4(iPosition, 1.0)).xyww;
}
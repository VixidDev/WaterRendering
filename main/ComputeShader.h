#pragma once

#include "glad/glad.h"
#include "ShaderManager.h"
#include "error.h"

#include <string>
#include <format>

class ComputeShader {
public:
	ComputeShader();
	ComputeShader(std::string computeFilename);

	GLuint _programID;
	GLuint _shaderID;
};


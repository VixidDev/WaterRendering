#pragma once

#include <string>
#include <format>

#include "glad/glad.h"
#include "ShaderManager.h"
#include "../utils/error.h"

class ComputeShader {
public:
	ComputeShader();
	ComputeShader(std::string computeFilename);

	GLuint _programID;
	GLuint _shaderID;
};


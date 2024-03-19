#include "Shader.h"
#include "ShaderManager.h"
#include "error.h"
#include "checkpoint.h"

#include <format>

Shader::Shader() {}

Shader::Shader(std::string vertexFileName, std::string fragmentFileName) :
	vertexFile(vertexFileName),
	fragmentFile(fragmentFileName)
{
	compile();
}

void Shader::compile() {
	OGL_CHECKPOINT_ALWAYS();

	deleteShaders();

	this->shaderProgram = glCreateProgram();

	OGL_CHECKPOINT_ALWAYS();

	this->vertexShader = ShaderManager::loadShader(GL_VERTEX_SHADER, this->vertexFile);
	glAttachShader(this->shaderProgram, this->vertexShader);

	OGL_CHECKPOINT_ALWAYS();

	this->fragmentShader = ShaderManager::loadShader(GL_FRAGMENT_SHADER, this->fragmentFile);
	glAttachShader(this->shaderProgram, this->fragmentShader);

	OGL_CHECKPOINT_ALWAYS();

	glLinkProgram(this->shaderProgram);

	OGL_CHECKPOINT_ALWAYS();

	GLint linkStatus;
	GLint logLength;
	glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &linkStatus);

	OGL_CHECKPOINT_ALWAYS();

	glGetProgramiv(this->shaderProgram, GL_INFO_LOG_LENGTH, &logLength);

	OGL_CHECKPOINT_ALWAYS();

	if (linkStatus == GL_FALSE) {
		std::string errorMessage = std::format("Failed to link shaders %s and %s. ", this->vertexFile, this->fragmentFile);

		if (logLength > 0) {
			std::vector<char> logMessage(logLength + 1);
			glGetShaderInfoLog(this->shaderProgram, logLength, nullptr, &logMessage[0]);
			throw Error("%s %s\n", errorMessage, &logMessage[0]);
		}
	}

	OGL_CHECKPOINT_ALWAYS();

	this->created = true;
}

void Shader::deleteShaders() {
	if (this->vertexShader >= 0) {
		glDeleteShader(this->vertexShader);
		this->vertexShader = -1;
	}

	if (this->fragmentShader >= 0) {
		glDeleteShader(this->fragmentShader);
		this->fragmentShader = -1;
	}

	if (this->shaderProgram >= 0) {
		glDeleteProgram(this->shaderProgram);
		this->shaderProgram = -1;
	}
}

void Shader::enable() {
	glUseProgram(this->shaderProgram);
}

void Shader::disable() {
	glUseProgram(0);
}

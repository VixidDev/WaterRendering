#include "Shader.h"
#include "ShaderManager.h"
#include "error.h"

Shader::Shader() {}

Shader::Shader(std::string vertexFileName, std::string fragmentFileName) :
	vertexFile(vertexFileName),
	fragmentFile(fragmentFileName)
{
	compile();
}

void Shader::compile() {
	deleteShaders();

	shaderProgram = glCreateProgram();

	vertexShader = ShaderManager::loadShader(GL_VERTEX_SHADER, vertexFile);
	glAttachShader(shaderProgram, vertexShader);

	fragmentShader = ShaderManager::loadShader(GL_FRAGMENT_SHADER, fragmentFile);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	GLint linkStatus;
	GLint logLength;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);

	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);

	if (linkStatus == GL_FALSE) {
		std::string errorMessage = std::format("Failed to link shaders %s and %s. ", vertexFile, fragmentFile);

		if (logLength > 0) {
			std::vector<char> logMessage(logLength + 1);
			glGetShaderInfoLog(shaderProgram, logLength, nullptr, &logMessage[0]);
			throw Error("%s %s\n", errorMessage, &logMessage[0]);
		}
	}

	created = true;
}

void Shader::deleteShaders() {
	if (vertexShader >= 0) {
		glDeleteShader(vertexShader);
		vertexShader = -1;
	}

	if (fragmentShader >= 0) {
		glDeleteShader(fragmentShader);
		fragmentShader = -1;
	}

	if (shaderProgram >= 0) {
		glDeleteProgram(shaderProgram);
		shaderProgram = -1;
	}
}

void Shader::enable() {
	glUseProgram(shaderProgram);
}

void Shader::disable() {
	glUseProgram(0);
}

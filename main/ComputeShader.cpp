#include "ComputeShader.h"

ComputeShader::ComputeShader() {}

ComputeShader::ComputeShader(std::string computeFilename) {
	_shaderID = ShaderManager::loadShader(GL_COMPUTE_SHADER, computeFilename);
	_programID = glCreateProgram();
	glAttachShader(_programID, _shaderID);
	glLinkProgram(_programID);

	GLint linkStatus;
	GLint logLength;
	glGetProgramiv(_programID, GL_LINK_STATUS, &linkStatus);

	glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &logLength);

	if (linkStatus == GL_FALSE) {
		std::string errorMessage = std::format("Failed to link shader %s. ", computeFilename);

		if (logLength > 0) {
			std::vector<char> logMessage(logLength + 1);
			glGetShaderInfoLog(_programID, logLength, nullptr, &logMessage[0]);
			throw Error("%s %s\n", errorMessage, &logMessage[0]);
		}
	}
}

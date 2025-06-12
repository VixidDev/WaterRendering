#include "ShaderManager.h"
#include "../utils/error.h"

std::map<std::string, Shader> ShaderManager::shaders;

void ShaderManager::initialiseShaders() {
	shaders.emplace("PBR", PBRShader());
	shaders.emplace("Skybox", SkyboxShader());
}

GLuint ShaderManager::loadShader(GLenum shaderType, const std::string &fileName) {	
	// Read shader file
	std::string shaderSource;
	std::ifstream shaderFile(fileName, std::ios::in);

	if (shaderFile.is_open()) {
		std::stringstream buffer;
		buffer << shaderFile.rdbuf();
		shaderSource = buffer.str();
	}
	else {
		throw Error("Could not open shader file: %s\n", fileName);
	}

	// Compile shader
	GLuint shaderID = glCreateShader(shaderType);
	char const* shaderSourcePointer = shaderSource.c_str();

	glShaderSource(shaderID, 1, &shaderSourcePointer, nullptr);
	glCompileShader(shaderID);

	// Check shader compilation status
	GLint compilationStatus;
	GLint logLength;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compilationStatus);
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);

	if (compilationStatus == GL_FALSE) {
		std::string errorMessage = std::format("Failed to compile shader %s. ", fileName.c_str());

		if (logLength > 0) {
			std::vector<char> logMessage(logLength + 1);
			glGetShaderInfoLog(shaderID, logLength, nullptr, &logMessage[0]);
			throw Error("%s %s\n", errorMessage, &logMessage[0]);
		}
	}

	return shaderID;
}

void ShaderManager::enableShader(std::string shaderName) {
	// Try to get the requested shader instance
	try {
		Shader shader = shaders.at(shaderName);
		shader.enable();
	}
	// If shader isn't in map
	catch (std::out_of_range error) {
		throw Error("Cannot get shader: %s\n", shaderName);
	}
}

Shader ShaderManager::getShaderInstance(std::string shaderName) {
	return shaders.at(shaderName);
}

void ShaderManager::disableShader() {
	glUseProgram(0);
}

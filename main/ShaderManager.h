#pragma once

#include <glad/glad.h>

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <any>

#include "Shader.h"

class ShaderManager {
public:
	static void initialiseShaders();

	static GLuint loadShader(GLenum shaderType, const std::string &fileName);
	static void enableShader(std::string shaderName);
	static void disableShader();
private:
	static std::map<std::string, Shader> shaders;
};
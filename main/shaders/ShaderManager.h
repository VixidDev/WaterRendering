#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <any>
#include <sstream>
#include <fstream>
#include <format>

#include <glad/glad.h>

#include "Shader.h"

class PBRShader : public Shader {
public:
	PBRShader() : Shader("../shaders/PBR.vert", "../shaders/PBR.frag") {}
};

class SkyboxShader : public Shader {
public:
	SkyboxShader() : Shader("../shaders/Skybox.vert", "../shaders/Skybox.frag") {}
};

class ShaderManager {
public:
	static void initialiseShaders();

	static GLuint loadShader(GLenum shaderType, const std::string &fileName);
	static void enableShader(std::string shaderName);
	static void disableShader();
	static Shader getShaderInstance(std::string shaderName);
private:
	static std::map<std::string, Shader> shaders;
};
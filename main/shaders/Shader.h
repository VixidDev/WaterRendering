#pragma once

#include <string>
#include <format>

class Shader {
public:
	Shader();
	Shader(std::string vertexFileName, std::string fragmentFileName);

	void compile();
	void deleteShaders();

	void enable();
	void disable();

	bool created = false;
	int shaderProgram = -1;
private:
	std::string vertexFile;
	std::string fragmentFile;

	int vertexShader = -1;
	int fragmentShader = -1;
};
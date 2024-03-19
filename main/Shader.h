#pragma once

#include <string>

class Shader {
public:
	Shader();
	Shader(std::string vertexFileName, std::string fragmentFileName);

	void compile();
	void deleteShaders();

	void enable();
	void disable();

	bool created = false;
private:
	std::string vertexFile;
	std::string fragmentFile;

	int shaderProgram = -1;
	int vertexShader = -1;
	int fragmentShader = -1;
};
#pragma once

#include <string>
#include <vector>

#include "stb_image.h"
#include "glad/glad.h"

#include "error.h"

class Skybox {
public:
	Skybox(std::vector<std::string> faces);

	void createVAO();

	GLuint _skyboxTexture;
	GLuint _skyboxVAO;
private:
	GLuint _skyboxPosVBO;
};


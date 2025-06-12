#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glad/glad.h"

struct Mesh {
	std::vector<glm::vec3> positions;
	std::vector<unsigned int> indices;
};

class OceanMesh {
public:
	static void initialiseMesh(int size);
	static void createPlaneMesh(int width, int height);
	static void createVAO();
	static GLuint getMeshVAO();
	static Mesh getMesh();

private:
	static int _width;
	static int _height;
	static GLuint _posVBO;
	static GLuint _indicesEBO;
	static GLuint _meshVAO;
	static Mesh _mesh;
};


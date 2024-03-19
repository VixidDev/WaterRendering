#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glad/glad.h"

#include "Waves.h"

struct Mesh {
	std::vector<glm::vec3> positions;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> colours;
	std::vector<glm::vec3> normals;
};

class OceanMesh {
public:
	static void initialiseMesh(Waves wave);
	static void createPlaneMesh(int width, int height);
	static void createVAO();
	static GLuint getMeshVAO();
	static Mesh getMesh();

private:
	static int _width;
	static int _height;
	static GLuint _posVBO;
	static GLuint _indicesEBO;
	static GLuint _colourVBO;
	static GLuint _normalVBO;
	static GLuint _meshVAO;
	static Mesh _mesh;
};


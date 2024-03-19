#include "OceanMesh.h"

Mesh OceanMesh::_mesh = Mesh();
GLuint OceanMesh::_posVBO = 0;
GLuint OceanMesh::_indicesEBO = 0;
GLuint OceanMesh::_colourVBO = 0;
GLuint OceanMesh::_normalVBO = 0;
GLuint OceanMesh::_meshVAO = 0;
int OceanMesh::_width = 0;
int OceanMesh::_height = 0;

void OceanMesh::initialiseMesh(Waves wave) {
	int size = wave.getSize();
	createPlaneMesh(size, size);
}

void OceanMesh::createPlaneMesh(int width, int height) {
	_mesh = Mesh();
	_width = width;
	_height = height;

	for (int z = 0; z < height; z++) {
		for (int x = 0; x < width; x++) {
			_mesh.positions.push_back(glm::vec3(x, 0, z));
			_mesh.colours.push_back(glm::vec3(1, 1, 1));
			_mesh.normals.push_back(glm::vec3(0, 1, 0));
		}
	}

	for (int z = 0; z < height - 1; z++) {
		for (int x = 0; x < width - 1; x++) {
			_mesh.indices.push_back(z * width + x);
			_mesh.indices.push_back((z + 1) * width + x);
			_mesh.indices.push_back((z + 1) * width + (x + 1));

			_mesh.indices.push_back(z * width + x);
			_mesh.indices.push_back((z + 1) * width + (x + 1));
			_mesh.indices.push_back(z * width + (x + 1));
		}
	}
}

void OceanMesh::createVAO() {
	glGenVertexArrays(1, &_meshVAO);
	glBindVertexArray(_meshVAO);

	glGenBuffers(1, &_posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _posVBO);
	glBufferData(GL_ARRAY_BUFFER, _mesh.positions.size() * sizeof(glm::vec3), _mesh.positions.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &_indicesEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indicesEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(unsigned int), _mesh.indices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &_colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _colourVBO);
	glBufferData(GL_ARRAY_BUFFER, _mesh.colours.size() * sizeof(glm::vec3), _mesh.colours.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &_normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _normalVBO);
	glBufferData(GL_ARRAY_BUFFER, _mesh.normals.size() * sizeof(glm::vec3), _mesh.normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, _posVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, _colourVBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, _normalVBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint OceanMesh::getMeshVAO() {
	return _meshVAO;
}

Mesh OceanMesh::getMesh() {
	return _mesh;
}
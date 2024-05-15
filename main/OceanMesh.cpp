#include "OceanMesh.h"

Mesh OceanMesh::_mesh = Mesh();
GLuint OceanMesh::_posVBO = 0;
GLuint OceanMesh::_indicesEBO = 0;
GLuint OceanMesh::_meshVAO = 0;
int OceanMesh::_width = 0;
int OceanMesh::_height = 0;

void OceanMesh::initialiseMesh(int size) {
	createPlaneMesh(size, size);
}

void OceanMesh::createPlaneMesh(int width, int height) {
	_mesh = Mesh();
	_width = width;
	_height = height;

	for (int z = 0; z < height; z++) {
		for (int x = 0; x < width; x++) {
			// Markus says we don't actually need to pass positions in to the gpu, but rather can use VertexID in the vertex shader
			// for positions, but since i've got it to work how it is, ill leave it for now
			_mesh.positions.push_back(glm::vec3(x, 0, z));
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

	glBindBuffer(GL_ARRAY_BUFFER, _posVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint OceanMesh::getMeshVAO() {
	return _meshVAO;
}

Mesh OceanMesh::getMesh() {
	return _mesh;
}
#pragma once

#include <map>

#include "Camera.h" // Inlcudes glm.hpp
#include "OceanMesh.h" // Includes Waves.h

struct GlobalState {
	Camera camera;
	Mesh mesh;
	GLuint meshVAO;

	float timeDelta;

	std::map<int, std::pair<bool, int>> keys;
};

#pragma once

#include <map>

#include "utils/Camera.h" // Inlcudes glm.hpp
#include "waves/OceanMesh.h" // Includes Waves.h
#include "waves/Waves.h"

struct GlobalState {
	Camera camera;
	Mesh mesh;
	GLuint meshVAO;

	float timeDelta = 0.f;

	std::map<int, std::pair<bool, int>> keys;
};

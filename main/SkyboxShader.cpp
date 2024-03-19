#include "ShaderManager.h"

class SkyboxShader : public Shader {
public:
	SkyboxShader() : Shader("../shaders/Skybox.vert", "../shaders/Skybox.frag") {}
};
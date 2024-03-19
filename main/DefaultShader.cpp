#include "ShaderManager.h"

class DefaultShader : public Shader {
public:
	DefaultShader() : Shader("../shaders/Water.vert", "../shaders/Water.frag") {}
};
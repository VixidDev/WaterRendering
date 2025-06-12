#pragma once

#include "../shaders/ComputeShader.h"

class FastFourierTransform {
public:
	FastFourierTransform();
	FastFourierTransform(int size);

	void TwiddlesAndIndices();
	void IFFT2D(GLuint inputTexture, GLuint bufferTexture);

	ComputeShader _butterfly;
	ComputeShader _fft;
	ComputeShader _permute;

	GLuint _butterflyTexture;

	int _size;
};
#pragma once

#include "ComputeShader.h"

class FastFourierTransform {
public:
	FastFourierTransform();
	FastFourierTransform(int size);

	void CalculateTwiddleFactorsAndInputIndices();
	void IFFT2D(GLuint inputTexture, GLuint bufferTexture, GLuint temp);

	ComputeShader _butterfly;
	ComputeShader _fft;
	ComputeShader _permute;

	GLuint _butterflyTexture;

	int _size;
};
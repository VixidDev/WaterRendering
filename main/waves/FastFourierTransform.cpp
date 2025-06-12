#include "FastFourierTransform.h"

FastFourierTransform::FastFourierTransform() {}

FastFourierTransform::FastFourierTransform(int size) : _size(size) {
	_butterfly = ComputeShader("../shaders/Butterfly.comp");
	_fft =		 ComputeShader("../shaders/FFT.comp");
	_permute =	 ComputeShader("../shaders/Permute.comp");

	TwiddlesAndIndices();
}

void FastFourierTransform::TwiddlesAndIndices() {
	int logSize = std::log2(_size);

	glGenTextures(1, &_butterflyTexture);
	glBindTexture(GL_TEXTURE_2D, _butterflyTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, logSize, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glUseProgram(_butterfly._programID);

	GLint sizeUniform = glGetUniformLocation(_butterfly._programID, "size");

	glUniform1i(sizeUniform, _size);

	glBindImageTexture(0, _butterflyTexture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glDispatchCompute(logSize, _size / 8, 1);
}

void FastFourierTransform::IFFT2D(GLuint inputTexture, GLuint bufferTexture) {
	int logSize = (int)std::log2(_size);
	int pingPong = 0;

	glUseProgram(_fft._programID);

	GLint pingPongUniform = glGetUniformLocation(_fft._programID, "pingpong");
	GLint stepUniform = glGetUniformLocation(_fft._programID, "iteration");
	GLint directionUniform = glGetUniformLocation(_fft._programID, "direction");

	glBindImageTexture(0, _butterflyTexture, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, inputTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, bufferTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

	for (int i = 0; i < logSize; i++) {
		pingPong++;
		pingPong %= 2;

		glUniform1i(pingPongUniform, pingPong);
		glUniform1i(stepUniform, i);
		glUniform1i(directionUniform, 0);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glDispatchCompute(_size / 8, _size / 8, 1);
	}

	glBindImageTexture(0, _butterflyTexture, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, inputTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, bufferTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

	for (int i = 0; i < logSize; i++) {
		pingPong++;
		pingPong %= 2;

		glUniform1i(pingPongUniform, pingPong);
		glUniform1i(stepUniform, i);
		glUniform1i(directionUniform, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glDispatchCompute(_size / 8, _size / 8, 1);
	}

	glUseProgram(_permute._programID);
	glBindImageTexture(0, inputTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);
	glUseProgram(0);
}

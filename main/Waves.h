#pragma once

#include <random>

#include "glm/glm.hpp"
#include "glad/glad.h"

#include "FastFourierTransform.h"

class Waves {
public:
	Waves();
	Waves(int size, FastFourierTransform fft);

	float jonswapPeakFrequency(float g, float fetch, float windspeed);
	float jonswapAlpha(float g, float fetch, float windspeed);
	void init();
	void calculateWaveSpectra();
	void calculateConjugateSpectra();
	void calculateWavesAtTime(float time);
	int getSize();

	float _gravity = 9.81f;
	float _fetch = 100000.0f;
	int _lengthScale = 256;
	float _depth = 500.0f;
	float _windSpeed = 7.29f;
	float _peakOmega;
	float _alpha;

	ComputeShader _waveSpectra;
	ComputeShader _waveSpectraConjugate;
	ComputeShader _timeDependentSpectra;
	ComputeShader _textureMerger;

	GLuint _h0kTexture;
	GLuint _h0Texture;
	GLuint _waveDataTexture;

	GLuint _Dx_DzTexture;
	GLuint _Dy_DxzTexture;
	GLuint _Dyx_DyzTexture;
	GLuint _Dxx_DzzTexture;

	GLuint _displacementTexture;
	GLuint _derivativesTexture;

	GLuint temp;

	FastFourierTransform _fft;

private:
	int _size;
};

Waves initialise(int size);
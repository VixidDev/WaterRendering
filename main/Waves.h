#pragma once

#include <random>
#include <array>

#include "glm/glm.hpp"
#include "glad/glad.h"

#include "WaveData.h"
#include "FastFourierTransform.h"

class Waves {
public:
	Waves();
	Waves(int size, FastFourierTransform fft);

	float jonswapPeakFrequency(float g, float fetch, float windspeed);
	float jonswapAlpha(float g, float fetch, float windspeed);
	void init(int lengthScale, float cutoffLow, float cutoffHigh);
	void calculateWaveSpectrum(int sScale, float cutoffLow, float cutoffHigh);
	void calculateConjugateSpectrum();
	void calculateWavesAtTime(float time, float timeDelta);
	void recalculateInitials(WaveData waveData, int scale, float cutoffLow, float cutoffHigh);

	float _gravity = 9.81f;
	float _fetch = 100000.0f;
	int _scale = 250;
	float _depth = 500.0f;
	float _windSpeed = 7.29f;
	float _windDirection = 29.81f;
	float _peakOmega;
	float _alpha;

	ComputeShader _waveSpectra;
	ComputeShader _waveSpectraConjugate;
	ComputeShader _timeDependentSpectra;
	ComputeShader _textureAssembler;

	GLuint _h0kTexture = -1;
	GLuint _h0Texture = -1;
	GLuint _waveDataTexture = -1;

	GLuint _choppinessTexture = -1;
	GLuint _elevationTexture = -1;
	GLuint _slopeParamsTexture = -1;
	GLuint _jacobianParamsTexture = -1;

	GLuint _displacementTexture = -1;
	GLuint _derivativesTexture = -1;
	GLuint _foamTexture = -1;

	FastFourierTransform _fft;

private:
	int _size;
};

std::array<Waves, 3> initialise(WaveData waveData, int size);
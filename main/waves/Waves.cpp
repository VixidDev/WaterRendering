#include "Waves.h"
#include <numeric>

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0.0, 1.0);

const float PI = 3.14159274f;

GLuint noiseID;

// Generates a size x size sized texture filled with Gaussian Distributed Random numbers
void generateGaussianNoise(int size) {
	float* data = (float*)calloc(size * size * 4, sizeof(float));

	if (data != NULL) {
		glGenTextures(1, &noiseID);

		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				data[(size * i + j) * 4 + 0] = std::cos(2 * PI * distribution(generator)) * std::sqrt(-2 * std::log(distribution(generator)));
				data[(size * i + j) * 4 + 1] = std::cos(2 * PI * distribution(generator)) * std::sqrt(-2 * std::log(distribution(generator)));
				data[(size * i + j) * 4 + 2] = 0;
				data[(size * i + j) * 4 + 3] = 0;
			}
		}

		glBindTexture(GL_TEXTURE_2D, noiseID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, data);
	}
}

Waves::Waves() {}

Waves::Waves(int size, FastFourierTransform fft) : _size(size), _fft(fft) {

	_peakOmega = jonswapPeakFrequency(_gravity, _fetch, _windSpeed);
	_alpha = jonswapAlpha(_gravity, _fetch, _windSpeed);

	_waveSpectra = ComputeShader("../shaders/WaveSpectra.comp");
	_waveSpectraConjugate = ComputeShader("../shaders/WaveSpectraConjugate.comp");
	_timeDependentSpectra = ComputeShader("../shaders/TimeDependentSpectra.comp");
	_textureAssembler = ComputeShader("../shaders/TextureAssembler.comp");
}

float Waves::jonswapPeakFrequency(float g, float fetch, float windspeed) {
	return 22 * std::pow(windspeed * fetch / g / g, -0.33f);
}

float Waves::jonswapAlpha(float g, float fetch, float windspeed) {
	return 0.076f * std::pow(g * fetch / windspeed / windspeed, -0.22f);
}

std::vector<float> timings1;
std::vector<float> timings2;
std::vector<float> timings3;

void Waves::init(int scale, float edgeLow, float edgeHigh) {
	_scale = scale;
	calculateWaveSpectrum(scale, edgeLow, edgeHigh);
	calculateConjugateSpectrum();

	// Initialise all textures used in the compute shaders
	glGenTextures(1, &_choppinessTexture);
	glBindTexture(GL_TEXTURE_2D, _choppinessTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_elevationTexture);
	glBindTexture(GL_TEXTURE_2D, _elevationTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_slopeParamsTexture);
	glBindTexture(GL_TEXTURE_2D, _slopeParamsTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_jacobianParamsTexture);
	glBindTexture(GL_TEXTURE_2D, _jacobianParamsTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_displacementTexture);
	glBindTexture(GL_TEXTURE_2D, _displacementTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_derivativesTexture);
	glBindTexture(GL_TEXTURE_2D, _derivativesTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_foamTexture);
	glBindTexture(GL_TEXTURE_2D, _foamTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Waves::calculateWaveSpectrum(int scale, float edgeLow, float edgeHigh) {
	if (_h0kTexture == -1) {
		glGenTextures(1, &_h0kTexture);
		glBindTexture(GL_TEXTURE_2D, _h0kTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	
	if (_waveDataTexture == -1) {
		glGenTextures(1, &_waveDataTexture);
		glBindTexture(GL_TEXTURE_2D, _waveDataTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	glUseProgram(_waveSpectra._programID);

	glUniform1i(0, scale);
	glUniform1i(1, _size);
	glUniform1f(2, _gravity);
	glUniform1f(3, _depth);
	glUniform1f(4, _peakOmega);
	glUniform1f(5, _alpha);
	glUniform1f(6, _windSpeed);
	glUniform1f(7, _windDirection);
	glUniform1f(8, edgeLow);
	glUniform1f(9, edgeHigh);

	glBindImageTexture(0, _h0kTexture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(1, _waveDataTexture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, noiseID, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);
}

void Waves::calculateConjugateSpectrum() {
	if (_h0Texture == -1) {
		glGenTextures(1, &_h0Texture);
		glBindTexture(GL_TEXTURE_2D, _h0Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	
	glUseProgram(_waveSpectraConjugate._programID);

	GLint sizeUniform = glGetUniformLocation(_waveSpectraConjugate._programID, "size");

	glUniform1i(sizeUniform, _size);

	glBindImageTexture(0, _h0kTexture, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, _h0Texture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);
}

void Waves::calculateWavesAtTime(float time, float timeDelta) {
	glUseProgram(_timeDependentSpectra._programID);

	GLint timeUniform = glGetUniformLocation(_timeDependentSpectra._programID, "time");

	glUniform1f(timeUniform, time);

	glBindImageTexture(0, _choppinessTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, _elevationTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, _slopeParamsTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, _jacobianParamsTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(4, _h0Texture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, _waveDataTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);

	_fft.IFFT2D(_choppinessTexture, _h0kTexture);
	_fft.IFFT2D(_elevationTexture, _h0kTexture);
	_fft.IFFT2D(_slopeParamsTexture, _h0kTexture);
	_fft.IFFT2D(_jacobianParamsTexture, _h0kTexture);

	glUseProgram(_textureAssembler._programID);

	GLint timeDeltaUniform = glGetUniformLocation(_textureAssembler._programID, "timeDelta");

	glUniform1f(timeDeltaUniform, timeDelta);

	glBindImageTexture(0, _displacementTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, _derivativesTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, _foamTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, _choppinessTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(4, _elevationTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, _slopeParamsTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(6, _jacobianParamsTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);
	glUseProgram(0);
}

void Waves::recalculateInitials(WaveData waveData, int scale, float cutoffLow, float cutoffHigh) {
	_peakOmega = jonswapPeakFrequency(waveData.gravity, waveData.fetch, waveData.windSpeed);
	_alpha = jonswapAlpha(waveData.gravity, waveData.fetch, waveData.windSpeed);
	_windDirection = waveData.angle;

	calculateWaveSpectrum(scale, cutoffLow, cutoffHigh);
	calculateConjugateSpectrum();
}

std::array<Waves, 3> initialise(WaveData waveData, int size) {
	FastFourierTransform fft = FastFourierTransform(size);

	generateGaussianNoise(size);

	Waves waves1 = Waves(size, fft);
	Waves waves2 = Waves(size, fft);
	Waves waves3 = Waves(size, fft);
	
	float edge1 = 2 * PI / waveData.scale2 * 10.0f;
	float edge2 = 2 * PI / waveData.scale3 * 10.0f;

	waves1.init(waveData.scale1, 0.0001f, edge1);
	waves2.init(waveData.scale2, edge1, edge2);
	waves3.init(waveData.scale3, edge2, 9999.9f);

	std::array<Waves, 3> waves = { waves1, waves2, waves3 };
	return waves;
}
#include "Waves.h"

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
	_textureMerger = ComputeShader("../shaders/TextureMerger.comp");
}

float Waves::jonswapPeakFrequency(float g, float fetch, float windspeed) {
	return 22 * std::pow(windspeed * fetch / g / g, -0.33f);
}

float Waves::jonswapAlpha(float g, float fetch, float windspeed) {
	return 0.076f * std::pow(g * fetch / windspeed / windspeed, -0.22f);
}

void Waves::init() {
	calculateWaveSpectra();
	calculateConjugateSpectra();

	// Initialise all textures used in the compute shaders
	glGenTextures(1, &_Dx_DzTexture);
	glBindTexture(GL_TEXTURE_2D, _Dx_DzTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_Dy_DxzTexture);
	glBindTexture(GL_TEXTURE_2D, _Dy_DxzTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_Dyx_DyzTexture);
	glBindTexture(GL_TEXTURE_2D, _Dyx_DyzTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_Dxx_DzzTexture);
	glBindTexture(GL_TEXTURE_2D, _Dxx_DzzTexture);
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

	glGenTextures(1, &temp);
	glBindTexture(GL_TEXTURE_2D, temp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Waves::calculateWaveSpectra() {
	glGenTextures(1, &_h0kTexture);
	glBindTexture(GL_TEXTURE_2D, _h0kTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &_waveDataTexture);
	glBindTexture(GL_TEXTURE_2D, _waveDataTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glUseProgram(_waveSpectra._programID);

	GLint lengthScaleUniform = glGetUniformLocation(_waveSpectra._programID, "lengthScale");
	GLint sizeUniform = glGetUniformLocation(_waveSpectra._programID, "size");
	GLint gravityUniform = glGetUniformLocation(_waveSpectra._programID, "gravity");
	GLint depthUniform = glGetUniformLocation(_waveSpectra._programID, "depth");
	GLint peakOmegaUniform = glGetUniformLocation(_waveSpectra._programID, "peakOmega");
	GLint alphaUniform = glGetUniformLocation(_waveSpectra._programID, "alpha");
	GLint windspeedUniform = glGetUniformLocation(_waveSpectra._programID, "windspeed");

	glUniform1i(lengthScaleUniform, _lengthScale);
	glUniform1i(sizeUniform, _size);
	glUniform1f(gravityUniform, _gravity);
	glUniform1f(depthUniform, _depth);
	glUniform1f(peakOmegaUniform, _peakOmega);
	glUniform1f(alphaUniform, _alpha);
	glUniform1f(windspeedUniform, _windSpeed);

	glBindImageTexture(0, _h0kTexture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(1, _waveDataTexture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, noiseID, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);
}

void Waves::calculateConjugateSpectra() {
	glGenTextures(1, &_h0Texture);
	glBindTexture(GL_TEXTURE_2D, _h0Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _size, _size, 0, GL_RGBA, GL_FLOAT, NULL);

	glUseProgram(_waveSpectraConjugate._programID);

	GLint sizeUniform = glGetUniformLocation(_waveSpectraConjugate._programID, "size");

	glUniform1i(sizeUniform, _size);

	glBindImageTexture(0, _h0kTexture, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, _h0Texture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);
}

void Waves::calculateWavesAtTime(float time) {
	glUseProgram(_timeDependentSpectra._programID);

	GLint timeUniform = glGetUniformLocation(_timeDependentSpectra._programID, "time");

	glUniform1f(timeUniform, time);

	glBindImageTexture(0, _Dx_DzTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, _Dy_DxzTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, _Dyx_DyzTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, _Dxx_DzzTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(4, _h0Texture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, _waveDataTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);

	_fft.IFFT2D(_Dx_DzTexture, _h0kTexture, temp);
	_fft.IFFT2D(_Dy_DxzTexture, _h0kTexture, temp);
	_fft.IFFT2D(_Dyx_DyzTexture, _h0kTexture, temp);
	_fft.IFFT2D(_Dxx_DzzTexture, _h0kTexture, temp);

	glUseProgram(_textureMerger._programID);

	glBindImageTexture(0, _displacementTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, _derivativesTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, _Dx_DzTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, _Dy_DxzTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(4, _Dyx_DyzTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, _Dxx_DzzTexture, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(_size / 8, _size / 8, 1);
}

int Waves::getSize() {
	return _size;
}

Waves initialise(int size) {
	FastFourierTransform fft = FastFourierTransform(size);

	generateGaussianNoise(size);

	Waves cascade = Waves(size, fft);
	cascade.init();

	return cascade;
}
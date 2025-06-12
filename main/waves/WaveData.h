#pragma once

extern struct WaveData {
	float depth = 500.0f;
	float windSpeed = 7.3f;
	float gravity = 9.81f;
	float fetch = 100000.0f;
	float angle = 39.55f;

	int scale1 = 250; // Large waves
	int scale2 = 19;
	int scale3 = 4; // Small waves
} waveData;
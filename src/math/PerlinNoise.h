#pragma once
#include <cstdint>

class PerlinNoise
{
public:
	PerlinNoise(uint64_t seed);

	float noise(float x, float y) const;
	float fbm(float x, float y, int octaves, float persistence) const;

private:
	float Fade(float t) const;
	float Lerp(float a, float b, float t) const;
	float Grad(int hash, float x, float y) const;

private:
	int m_Permutation[512];
};
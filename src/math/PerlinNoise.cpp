#include "PerlinNoise.h"
#include <cmath>
#include <random>

PerlinNoise::PerlinNoise(uint64_t seed)
{
	for (int i = 0; i < 256; i++)
	{
		m_Permutation[i] = i;
	}

	std::mt19937 rng(seed);

	// Shuffle the permutation array using the seed
	for (int i = 255; i > 0; i--)
	{
		std::uniform_int_distribution<int> dist(0, i);
		int j = dist(rng);
		std::swap(m_Permutation[i], m_Permutation[j]);
	}
	// Duplicate the permutation array
	for (int i = 0; i < 256; i++)
	{
		m_Permutation[256 + i] = m_Permutation[i];
	}
}

float PerlinNoise::noise(float x, float y) const
{
	// Determine grid cell coordinates
	int xi = static_cast<int>(std::floor(x)) & 255;
	int yi = static_cast<int>(std::floor(y)) & 255;

	// Relative x and y in the grid cell
	float xf = x - static_cast<int>(std::floor(x));
	float yf = y - static_cast<int>(std::floor(y));

	// Make interpolation not linear
	float u = Fade(xf);
	float v = Fade(yf);

	int aa = m_Permutation[m_Permutation[xi] + yi];
	int ab = m_Permutation[m_Permutation[xi] + yi + 1];
	int ba = m_Permutation[m_Permutation[xi + 1] + yi];
	int bb = m_Permutation[m_Permutation[xi + 1] + yi + 1];

	float x1 = Lerp(Grad(aa, xf, yf), Grad(ba, xf - 1, yf), u);
	float x2 = Lerp(Grad(ab, xf, yf - 1), Grad(bb, xf - 1, yf - 1), u);
	return (Lerp(x1, x2, v) + 1) / 2;
}

float PerlinNoise::fbm(float x, float y, int octaves, float persistence) const
{
	float total = 0.0f;
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float maxValue = 0.0f;

	for (int i = 0; i < octaves; i++)
	{
		total += noise(x * frequency, y * frequency) * amplitude;

		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2.0f;
	}
	return total / maxValue;
}

/* Private functions */

float PerlinNoise::Fade(float t) const
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::Lerp(float a, float b, float t) const
{
	return a + t * (b - a);
}

float PerlinNoise::Grad(int hash, float x, float y) const
{
	switch (hash & 0x03)
	{
	case 0: return x + y;
	case 1: return -x + y;
	case 2: return x - y;
	case 3: return -x - y;
	default: return 0;
	}
}
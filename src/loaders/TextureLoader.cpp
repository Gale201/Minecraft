#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

TextureLoader::TextureLoader(const std::string& texturesFolderPath)
	: m_TexturesFolderPath(texturesFolderPath)
{
	if (texturesFolderPath.back() != '/' && texturesFolderPath.back() != '\\')
	{
		m_TexturesFolderPath += '/';
	}
}

Texture TextureLoader::LoadTexture(const std::string& fileName)
{
	int width, height, channels;
	unsigned char* data = stbi_load(
		(m_TexturesFolderPath + fileName).c_str(),
		&width,
		&height,
		&channels,
		4
	);

	Texture texture(width, height);

	
	for (int i = 0; i < width * height; i++)
	{
		uint8_t r = data[i * 4];
		uint8_t g = data[i * 4 + 1];
		uint8_t b = data[i * 4 + 2];
		uint8_t a = data[i * 4 + 3];

		texture.SetPixelByIndex(i, (a << 24) | (r << 16) | (g << 8) | b);
	}

	stbi_image_free(data);

	return texture;
}
#pragma once
#include <string>
#include <optional>
#include "../render/Texture.h"

class TextureLoader
{
public:
	TextureLoader(const std::string& texturesFolderPath);
	~TextureLoader() = default;

	std::optional<Texture> LoadTexture(const std::string& fileName);

	std::string GetTexturesFolderPath()
	{
		return m_TexturesFolderPath;
	}

private:
	std::string m_TexturesFolderPath;
};

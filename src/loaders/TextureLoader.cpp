#include "TextureLoader.h"

#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"
#include "../core/Log.h"

namespace Minecraft
{
	TextureLoader::TextureLoader(const std::string& texturesFolderPath)
		: m_TexturesFolderPath(texturesFolderPath)
	{
		if (texturesFolderPath.back() != '/' && texturesFolderPath.back() != '\\')
		{
			m_TexturesFolderPath += '/';
		}
	}

	bool TextureLoader::LoadAllTextures()
	{
		for (const auto& [name, fileName] : m_TextureFileNames)
		{
			if (!LoadAndCheckTexture(name, fileName))
				return false;
		}

		return true;
	}

	std::optional<Texture> TextureLoader::LoadTexture(const std::string& fileName) const
	{
		if (!std::filesystem::exists(m_TexturesFolderPath + fileName)) return std::optional<Texture>();

		int width, height, channels;
		unsigned char* data = stbi_load(
			(m_TexturesFolderPath + fileName).c_str(),
			&width,
			&height,
			&channels,
			4
		);

		if (width < 1 || height < 1) return std::optional<Texture>();

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

	/* Private functions */

	bool TextureLoader::LoadAndCheckTexture(const std::string& name, const std::string& fileName)
	{
		auto texture = LoadTexture(fileName);
		if (texture.has_value())
			m_Textures[name] = std::make_unique<Texture>(texture.value());
		else
		{
			LOG_ERROR(("Texture '" + name + "' failed to load!").c_str());
			return false;
		}
	}

	void TextureLoader::SetOpacityForTexture(Texture& texture, float opacity)
	{
		for (int i = 0; i < texture.GetWidth() * texture.GetHeight(); i++)
		{
			uint32_t color = texture.GetPixels()[i];
			uint8_t a = (color >> 24) & 0xFF;
			color &= 0x00FFFFFF;

			float alpha = a / 255.0f;
			alpha = std::fmaxf(alpha - 1.0f + opacity, 0.0f);

			uint8_t newAlpha = (uint8_t)(alpha * 255.0f);
			color |= newAlpha << 24;

			texture.SetPixelByIndex(i, color);
		}
	}
}

#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <memory>
#include "../render/Texture.h"

class TextureLoader
{
public:
	TextureLoader(const std::string& texturesFolderPath);
	~TextureLoader() = default;

	bool LoadAllTextures();

	std::optional<Texture> LoadTexture(const std::string& fileName) const;

	bool Has(const std::string& name) const
	{
		auto it = m_Textures.find(name);
		if (it == m_Textures.end()) return false;
		return true;
	}

	Texture& Get(const std::string& name) const { return *m_Textures.at(name); }

	std::string GetTexturesFolderPath() const
	{
		return m_TexturesFolderPath;
	}
private:
	void SetOpacityForTexture(Texture& texture, float opacity);

private:
	std::string m_TexturesFolderPath;
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;
};

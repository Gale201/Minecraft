#pragma once
#include "../src/render/Texture.h"
#include <unordered_map>
#include <string>

namespace Minecraft
{
	class FontLoader
	{
	public:
		FontLoader(const std::string& characters)
			: m_Characters(characters)
		{
		}

		void LoadFont(const Texture& fontTexture)
		{
			m_FontTexture = &fontTexture;
			for (int i = 0; i < m_Characters.size(); i++)
			{
				int x = (i % 16) * (m_CharWidth + 2);
				int y = (2 + i / 16) * m_CharHeight;
				m_CharacterTextures[m_Characters[i]] = m_FontTexture->Crop(x, y, m_CharWidth, m_CharHeight);
			}
		}

		const Texture* GetCharacterTexture(char c) const
		{
			auto it = m_CharacterTextures.find(c);
			if (it != m_CharacterTextures.end())
				return &it->second;
			return nullptr;
		}

		int GetCharWidth() const { return m_CharWidth; }
		int GetCharHeight() const { return m_CharHeight; }

	private:
		const Texture* m_FontTexture = nullptr;
		const std::string m_Characters;
		int m_CharWidth = 6;
		int m_CharHeight = 8;
		std::unordered_map<char, Texture> m_CharacterTextures;
	};
}
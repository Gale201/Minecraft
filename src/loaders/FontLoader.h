#pragma once
#include "../render/Texture.h"
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
				int charWidth = GetCharVariableWidth(m_Characters[i]);
				m_VariableWidthCharacterTextures[m_Characters[i]] = m_FontTexture->Crop(x, y, charWidth, m_CharHeight);
			}
		}

		const Texture* GetCharacterTexture(char c) const
		{
			auto it = m_CharacterTextures.find(c);
			if (it != m_CharacterTextures.end())
				return &it->second;
			return nullptr;
		}

		const Texture* GetVariableWidthCharacterTexture(char c) const
		{
			auto it = m_VariableWidthCharacterTextures.find(c);
			if (it != m_VariableWidthCharacterTextures.end())
				return &it->second;
			return nullptr;
		}

		int GetCharWidth() const { return m_CharWidth; }
		int GetCharHeight() const { return m_CharHeight; }

	private:
		int GetCharVariableWidth(char c) const
		{
			switch (c)
			{
			case '!':
			case '.':
			case ':':
			case ';':
			case 'i':
			case '|':
				return 1;
			case 'l':
			case '\'':
				return 2;
			case '\\':
				return 0;
			case 't':
			case '[':
			case ']':
			case 'I':
				return 3;
			case '\"':
			case '(':
			case ')':
			case '{':
			case '}':
			case '<':
			case '>':
			case 'f':
			case 'k':
			case ' ':
				return 4;
			case '@':
			case '%':
			case '&':
			case '~':
				return 6;
			case '*':
				return 7;
			default:
				return 5;
			}
		}

	private:
		const Texture* m_FontTexture = nullptr;
		const std::string m_Characters;
		int m_CharWidth = 6;
		int m_CharHeight = 8;
		std::unordered_map<char, Texture> m_CharacterTextures;
		std::unordered_map<char, Texture> m_VariableWidthCharacterTextures;
	};
}

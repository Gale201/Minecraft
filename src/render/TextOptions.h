#pragma once

namespace Minecraft
{
	enum class TextAlignment
	{
		Left,
		Center,
		Right
	};

	struct TextOptions
	{
		TextAlignment alignment = TextAlignment::Left;
		bool variableWidth = true;
		bool drawShadow = true;
		int letterSpacing = 1;
		int rowSpacing = 1;
	};
}

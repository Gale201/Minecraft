#pragma once
#include <vector>

namespace Minecraft
{
    class Texture
    {
    public:
        Texture() : m_Width(0), m_Height(0) {};

        Texture(int width, int height)
            : m_Width(width), m_Height(height), m_Pixels(width* height)
        {
        }

        Texture Crop(int x, int y, int width, int height) const
        {
			Texture cropped(width, height);
            for (int i = x; i < x + width; i++)
            {
				for (int j = y; j < y + height; j++)
                {
                    cropped.SetPixel(i - x, j - y, GetPixel(i, j));
                }
            }
            return cropped;
        }

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

        uint32_t GetPixel(int x, int y) const
        {
            if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return m_DefaultColor;

            return m_Pixels[y * m_Width + x];
        }


        uint32_t GetPixelAtUV(float u, float v) const
        {
            int x = (int)(u * m_Width);
            int y = (int)(v * m_Height);

            return GetPixel(x, y);
        }
        
        const uint32_t* GetPixels() const
        {
            return m_Pixels.data();
        }

        void SetPixel(int x, int y, uint32_t color)
        {
            if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) return;
            m_Pixels[y * m_Width + x] = color;
        }

        void SetPixelByIndex(int i, uint32_t color)
        {
            if (i < 0 || i >= m_Width * m_Height) return;
            m_Pixels[i] = color;
        }

        void SetDefaultColor(uint32_t color)
        {
            m_DefaultColor = color;
        }

    private:
        int m_Width;
        int m_Height;
        std::vector<uint32_t> m_Pixels;

        uint32_t m_DefaultColor{ 0 };
    };
}

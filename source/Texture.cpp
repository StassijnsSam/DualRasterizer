#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//Load SDL_Surface using IMG_LOAD
		SDL_Surface* pSurface = IMG_Load(path.c_str());
		//Create & Return a new Texture Object (using SDL_Surface)
		return new Texture(pSurface);
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//Sample the correct texel for the given uv
		Uint32 pixelX = Uint32(m_pSurface->w * uv.x);
		Uint32 pixelY = Uint32(m_pSurface->h * uv.y);

		Uint32 pixelIndex = pixelY * m_pSurface->w + pixelX;
		Uint8 r{}, g{}, b{};
		SDL_GetRGB(m_pSurfacePixels[pixelIndex], m_pSurface->format, &r, &g, &b);

		ColorRGB texelColor{ float(r), float(g), float(b) };
		//put in range 0 to 1
		texelColor /= 255.f;
		return texelColor;
	}
}
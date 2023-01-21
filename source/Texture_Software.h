#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture_Software
	{
	public:
		~Texture_Software();

		static Texture_Software* LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;

	private:
		Texture_Software(SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}
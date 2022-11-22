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
		//Create & Return a new Texture Object (using SDL_Surface)
		Texture* pTexture{ new Texture(IMG_Load(path.c_str())) };
		return pTexture;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//Sample the correct texel for the given uv
		
		//uv is between [0,1] -> convert to [0, width] and [0, height]
		int x{ int(uv.x * m_pSurface->w) };
		int y{ int(uv.y * m_pSurface->h) };

		Uint32 pixelIdx = m_pSurfacePixels[x + (y * m_pSurface->w)];

		SDL_Color sdl_rgb;
		SDL_GetRGB(pixelIdx, m_pSurface->format, &sdl_rgb.r, &sdl_rgb.g, &sdl_rgb.b);
		ColorRGB rgb{ sdl_rgb.r / 255.f, sdl_rgb.g / 255.f, sdl_rgb.b / 255.f };
		return rgb;
	}
}
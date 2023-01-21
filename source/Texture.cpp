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
		if (m_pSurface != nullptr)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
		if (m_pResource != nullptr) {
			m_pResource->Release();
		}

		if (m_pResourceView != nullptr) {
			m_pResourceView->Release();
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
	ID3D11ShaderResourceView* Texture::GetResourceView()
	{
		return m_pResourceView;
	}
	ID3D11Texture2D* Texture::GetResource()
	{
		return m_pResource;
	}
	void Texture::CreateDirectXResources(ID3D11Device* pDevice)
	{
		auto pTextureSurface = m_pSurface;

		if (m_pSurface == nullptr) {
			throw std::runtime_error("Texture surface not found");
		}

		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pTextureSurface->w;
		desc.Height = pTextureSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = pTextureSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pTextureSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(pTextureSurface->h * pTextureSurface->pitch);

		auto* pResource = GetResource();
		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &pResource);
		if (FAILED(hr)) {
			// Delete texture to not get memory leaks
			throw std::runtime_error("Failed to create texture");
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		auto pResourceView = GetResourceView();
		hr = pDevice->CreateShaderResourceView(pResource, &SRVDesc, &pResourceView);
		if (FAILED(hr)) {
			// Delete texture to not get memory leaks
			throw std::runtime_error("Failed to create resource view for texture");
		}
	}
}
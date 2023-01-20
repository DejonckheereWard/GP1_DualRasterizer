#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <cassert>

using namespace dae;



Texture::~Texture()
{
	m_pShaderResourceView->Release();
	m_pResource->Release();

	SDL_FreeSurface(m_pSurface);
	//m_pSurface = nullptr;

}


// Static function
Texture* Texture::LoadFromFile(ID3D11Device* pDevice, const std::string& path)
{
	//TODO
	//Load SDL_Surface using IMG_LOAD
	//Create & Return a new Texture Object (using SDL_Surface)
	SDL_Surface* pSurface = IMG_Load(path.c_str());

	assert(pSurface != nullptr);
	
	return new Texture(pDevice, pSurface);
}

ColorRGB Texture::Sample(const Vector2& uv, UVMode uvMode) const
{

	int x = 0;
	int y = 0;

	switch(uvMode)
	{

		case UVMode::Wrap:
		{
			float uvX = uv.x;
			float uvY = uv.y;

			if(uvX < 0)
				uvX += abs(int(uvX)) + 1;
			if(uvY < 0)
				uvY += abs(int(uvY)) + 1;

			x = int(uvX * m_pSurface->w) % m_pSurface->w;
			y = int(uvY * m_pSurface->h) % m_pSurface->h;
			break;
		}

		case UVMode::Clamp:
			x = int(uv.x * m_pSurface->w);
			y = int(uv.y * m_pSurface->h);
			x = Clamp(x, 0, m_pSurface->w);
			y = Clamp(y, 0, m_pSurface->h);
			break;

		case UVMode::Mirror:
			x = int(uv.x * m_pSurface->w);
			y = int(uv.y * m_pSurface->h);
			if(x % 2 == 0)
				x = x % m_pSurface->w;
			else
				x = m_pSurface->w - (x % m_pSurface->w);
			break;

		case UVMode::Border:
			x = int(uv.x * m_pSurface->w);
			y = int(uv.y * m_pSurface->h);
			if(x < 0 || x >= m_pSurface->w || y < 0 || y >= m_pSurface->h)
				return ColorRGB{ 1.0f,0,1.0f };
			break;


		default:
			assert(false && "Shouldn't ever hit this in the switch");
			break;

	}

	// pixel color is in 0-255 ranges  0xFF FF FF FF -> ALPHA, BLUE, GREEN, RED
	const uint32_t pixelColor = m_pSurfacePixels[(y * m_pSurface->w) + x];

	ColorRGB color{};
	color.r = float((pixelColor >> 0) & 0xFF);  // 0 shift because RED is least significnat, 0xFF because we want to mask out the other colors (only take last byte)
	color.g = float((pixelColor >> 8) & 0xFF);
	color.b = float((pixelColor >> 16) & 0xFF);
	//color.a = (pixelColor >> 24) & 0xFF; // No A component apparently 

	color /= 255.0f;  // "Normalize" from 0->1

	return color;
}

Texture::Texture(ID3D11Device* pDevice, SDL_Surface* pSurface):
	m_pSurface{pSurface},
	m_pSurfacePixels{ (uint32_t*)pSurface->pixels}
{

	// Assemble the resource and shader resource view for directx
	DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT result = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
	if(FAILED(result))
	{
		std::cout << "Error creating Texture2D\n";
		assert(false);
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	result = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);

	if(FAILED(result))
	{
		std::cout << "Error creating Shader Resource View\n";
		assert(false);
	}

}

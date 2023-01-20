#pragma once

#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

using namespace dae;

class Texture final
{
public:
	enum class UVMode
	{
		Wrap,
		Mirror,
		Clamp,
		Border
	};

	~Texture();

	static Texture* LoadFromFile(ID3D11Device* pDevice, const std::string& path);
	ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView; };
	
	ColorRGB Sample(const Vector2& uv, UVMode uvMode = UVMode::Wrap) const;

private:
	Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);

	// Software
	SDL_Surface* m_pSurface{ nullptr };
	uint32_t* m_pSurfacePixels{ nullptr };

	// DirectX
	ID3D11Texture2D* m_pResource{};
	ID3D11ShaderResourceView* m_pShaderResourceView{};
};


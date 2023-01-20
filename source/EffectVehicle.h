#pragma once
#include "Matrix.h"
#include "Effect.h"

class Texture;

class EffectVehicle: public Effect
{
public:

	EffectVehicle(ID3D11Device* pDevice, const std::wstring& assetFile);

	virtual ~EffectVehicle();

	EffectVehicle(const EffectVehicle&) = delete;
	EffectVehicle& operator=(const EffectVehicle&) = delete;
	EffectVehicle(EffectVehicle&&) = delete;
	EffectVehicle& operator=(EffectVehicle&&) = delete;


	void SetDiffuseMap(Texture* pTexture);
	void SetNormalMap(Texture* pTexture);
	void SetSpecularMap(Texture* pTexture);
	void SetGlossinessMap(Texture* pTexture);


private:
	// Textures
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;




};
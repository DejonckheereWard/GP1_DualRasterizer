#pragma once
#include "Matrix.h"
#include "Effect.h"

class Texture;

using namespace dae;
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
	
	void SetLightDirection(Vector3 lightDirection);
	void SetLightIntensity(float lightIntensity);
	void SetLightColor(ColorRGB lightColor);
	void SetAmbientlight(ColorRGB ambientLight);
	void SetShininess(float shininess);


private:
	// Textures
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVar;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVar;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVar;

	//ID3DX11EffectMatrixVariable;
	// Light
	ID3DX11EffectVectorVariable* m_pLightDirectionVar;
	ID3DX11EffectScalarVariable* m_pLightIntensityVar;
	ID3DX11EffectVectorVariable* m_pLightColorVar;
	
	// Other scene settings
	ID3DX11EffectVectorVariable* m_pAmbientLightVar;
	ID3DX11EffectScalarVariable* m_pShininessVar;
	




};
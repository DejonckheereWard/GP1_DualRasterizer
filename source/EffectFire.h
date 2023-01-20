#pragma once
#include "Matrix.h"
#include "Effect.h" 


class Texture;

class EffectFire final: public Effect
{
public:

	enum class SamplerFilter
	{
		Point,
		Linear,
		Anisotropic
	};

	EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile);

	virtual ~EffectFire();
	EffectFire(const EffectFire&) = delete;
	EffectFire& operator=(const EffectFire&) = delete;
	EffectFire(EffectFire&&) = delete;
	EffectFire& operator=(EffectFire&&) = delete;

	ID3DX11Effect* GetEffect() const { return m_pEffect; };
	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };
	ID3DX11EffectMatrixVariable* GetMatrixVariable() const { return m_pMatWorldViewProjVariable; };
	ID3DX11EffectMatrixVariable* GetViewInverseMatrixVariable() const { return m_pEffectViewInverseMatrixVariable; };
	ID3DX11EffectMatrixVariable* GetWorldMatrixVariable() const { return m_pEffectWorldMatrixVariable; };


	void SetDiffuseMap(Texture* pTexture);

private:
	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;

	// Textures
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

	ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable;
	ID3DX11EffectMatrixVariable* m_pEffectWorldMatrixVariable;
	ID3DX11EffectMatrixVariable* m_pEffectViewInverseMatrixVariable;

	// Store the different filter modes for the sampler
	ID3D11SamplerState* m_pPointSampler;
	ID3D11SamplerState* m_pLinearSampler;
	ID3D11SamplerState* m_pAnisotropicSampler;

	SamplerFilter m_CurrentSamplerFilter;

};

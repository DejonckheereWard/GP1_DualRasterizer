#pragma once
class Texture;
class Effect
{
public:
	enum class SamplerFilter
	{
		Point,
		Linear,
		Anisotropic
	};

	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~Effect();

	Effect(const Effect&) = delete;
	Effect& operator=(const Effect&) = delete;
	Effect(Effect&&) = delete;
	Effect& operator=(Effect&&) = delete;

	ID3DX11Effect* GetEffect() const { return m_pEffect; };
	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };
	ID3DX11EffectMatrixVariable* GetWorldViewProjVariable() const { return m_pMatWorldViewProjVariable; };

	ID3DX11EffectMatrixVariable* GetViewInverseMatrixVariable() const { return m_pEffectViewInverseMatrixVariable; };
	ID3DX11EffectMatrixVariable* GetWorldMatrixVariable() const { return m_pEffectWorldMatrixVariable; };

	void SetSamplerFilter(SamplerFilter filter);

protected:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	ID3DX11EffectMatrixVariable* m_pEffectWorldMatrixVariable;
	ID3DX11EffectMatrixVariable* m_pEffectViewInverseMatrixVariable;


	// Store the different filter modes for the sampler
	ID3D11SamplerState* m_pPointSampler;
	ID3D11SamplerState* m_pLinearSampler;
	ID3D11SamplerState* m_pAnisotropicSampler;

	ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable;

	SamplerFilter m_CurrentSamplerFilter;

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};

#include "pch.h"
#include "EffectFire.h"
#include "Texture.h"

EffectFire::EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile):
	Effect(pDevice, assetFile)
{

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if(!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable is not valid!\n";
	}


	// Create the different sampler states with different filtesr
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;


}

EffectFire::~EffectFire()
{
	// Cleanup DirectX resources (do it in reverse of init)
	Utils::SafeRelease(m_pDiffuseMapVariable);

}

void EffectFire::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if(m_pDiffuseMapVariable)
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
}
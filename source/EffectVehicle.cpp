#include "pch.h"
#include "EffectVehicle.h"
#include "Texture.h"

EffectVehicle::EffectVehicle(ID3D11Device* pDevice, const std::wstring& assetFile):
	Effect(pDevice, assetFile)
{
	// TEXTURES
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if(!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable is not valid!\n";
	}

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if(!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"m_pNormalMapVariable is not valid!\n";
	}

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if(!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"m_pSpecularMapVariable is not valid!\n";
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if(!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"m_pGlossinessMapVariable is not valid!\n";
	}





}

EffectVehicle::~EffectVehicle()
{
	// Cleanup DirectX resources (do it in reverse of init)
	using namespace Utils;

	SafeRelease(m_pDiffuseMapVariable);

	SafeRelease(m_pGlossinessMapVariable);
	SafeRelease(m_pSpecularMapVariable);
	SafeRelease(m_pNormalMapVariable);

}

void EffectVehicle::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if(m_pDiffuseMapVariable)
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
}

void EffectVehicle::SetNormalMap(Texture* pTexture)
{
	if(m_pNormalMapVariable)
		m_pNormalMapVariable->SetResource(pTexture->GetShaderResourceView());
}

void EffectVehicle::SetSpecularMap(Texture* pTexture)
{
	if(m_pSpecularMapVariable)
		m_pSpecularMapVariable->SetResource(pTexture->GetShaderResourceView());
}

void EffectVehicle::SetGlossinessMap(Texture* pTexture)
{
	if(m_pGlossinessMapVariable)
		m_pGlossinessMapVariable->SetResource(pTexture->GetShaderResourceView());
}


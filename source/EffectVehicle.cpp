#include "pch.h"
#include "EffectVehicle.h"
#include "Texture.h"

EffectVehicle::EffectVehicle(ID3D11Device* pDevice, const std::wstring& assetFile):
	Effect(pDevice, assetFile)
{
	// TEXTURES
	m_pDiffuseMapVar = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if(!m_pDiffuseMapVar->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable is not valid!\n";
	}

	m_pNormalMapVar = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if(!m_pNormalMapVar->IsValid())
	{
		std::wcout << L"m_pNormalMapVariable is not valid!\n";
	}

	m_pSpecularMapVar = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if(!m_pSpecularMapVar->IsValid())
	{
		std::wcout << L"m_pSpecularMapVariable is not valid!\n";
	}

	m_pGlossinessMapVar = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if(!m_pGlossinessMapVar->IsValid())
	{
		std::wcout << L"m_pGlossinessMapVariable is not valid!\n";
	}

	
	// LIGHTS
	m_pLightDirectionVar = m_pEffect->GetVariableByName("gLightDirection")->AsVector();
	if(!m_pLightDirectionVar->IsValid())
	{
		std::wcout << L"m_pLightDirectionVariable is not valid!\n";
	}

	m_pLightColorVar = m_pEffect->GetVariableByName("gLightColor")->AsVector();
	if(!m_pLightColorVar->IsValid())
	{
		std::wcout << L"m_pLightColorVariable is not valid!\n";
	}	

	m_pLightIntensityVar = m_pEffect->GetVariableByName("gLightIntensity")->AsScalar();
	if(!m_pLightIntensityVar->IsValid())
	{
		std::wcout << L"m_pLightIntensityVar is not valid!\n";
	}

	m_pAmbientLightVar = m_pEffect->GetVariableByName("gAmbientColor")->AsVector();
	if(!m_pAmbientLightVar->IsValid())
	{
		std::wcout << L"m_pAmbientLightVar is not valid!\n";
	}

	// MATERIAL
	m_pShininessVar = m_pEffect->GetVariableByName("gShininess")->AsScalar();
	if(!m_pShininessVar->IsValid())
	{
		std::wcout << L"m_pShininessVar is not valid!\n";
	}
	





}

EffectVehicle::~EffectVehicle()
{
	// Cleanup DirectX resources (do it in reverse of init)
	using namespace Utils;

	SafeRelease(m_pDiffuseMapVar);
	SafeRelease(m_pGlossinessMapVar);
	SafeRelease(m_pSpecularMapVar);
	SafeRelease(m_pNormalMapVar);
	
	SafeRelease(m_pLightDirectionVar);
	SafeRelease(m_pLightIntensityVar);
	SafeRelease(m_pLightColorVar);
	SafeRelease(m_pAmbientLightVar);
	SafeRelease(m_pGlossinessMapVar);

}

void EffectVehicle::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if(m_pDiffuseMapVar)
		m_pDiffuseMapVar->SetResource(pDiffuseTexture->GetShaderResourceView());
}

void EffectVehicle::SetNormalMap(Texture* pTexture)
{
	if(m_pNormalMapVar)
		m_pNormalMapVar->SetResource(pTexture->GetShaderResourceView());
}

void EffectVehicle::SetSpecularMap(Texture* pTexture)
{
	if(m_pSpecularMapVar)
		m_pSpecularMapVar->SetResource(pTexture->GetShaderResourceView());
}

void EffectVehicle::SetGlossinessMap(Texture* pTexture)
{
	if(m_pGlossinessMapVar)
		m_pGlossinessMapVar->SetResource(pTexture->GetShaderResourceView());
}

void EffectVehicle::SetLightDirection(Vector3 lightDirection)
{
	if(m_pLightDirectionVar)
		m_pLightDirectionVar->SetRawValue(reinterpret_cast<float*>(&lightDirection), 0, sizeof(float)*3);
}

void EffectVehicle::SetLightIntensity(float lightIntensity)
{
	if(m_pLightIntensityVar)
		m_pLightIntensityVar->SetFloat(lightIntensity);
}

void EffectVehicle::SetLightColor(ColorRGB lightColor)
{
	if(m_pLightColorVar)
		m_pLightColorVar->SetRawValue(reinterpret_cast<float*>(&lightColor), 0, sizeof(float)*3);

	
}

void EffectVehicle::SetAmbientlight(ColorRGB ambientLight)
{
	if(m_pAmbientLightVar)
		m_pAmbientLightVar->SetRawValue(reinterpret_cast<float*>(&ambientLight), 0, sizeof(float)*3);
}

void EffectVehicle::SetShininess(float shininess)
{
	if(m_pShininessVar)
		m_pShininessVar->SetFloat(shininess);
}


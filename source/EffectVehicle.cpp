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

	// Rasterizer state (for culling setting)
	m_pRasterizerStateVar = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();
	if(!m_pRasterizerStateVar->IsValid())
	{
		std::wcout << L"m_pShininessVar is not valid!\n";
	}
	

	// Create the different Rasterizer States with different culling options
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	HRESULT hr;
	

	// Backface culling
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	hr = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateBFC);
	if(FAILED(hr))
		std::wcout << L"Failed to create Rasterizer State with backface culling" << std::endl;

	// Frontface culling
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	hr = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateFFC);
	if(FAILED(hr))
		std::wcout << L"Failed to create Rasterizer State with frontface culling" << std::endl;

	// NO culling
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	hr = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerStateNC);
	if(FAILED(hr))
		std::wcout << L"Failed to create Rasterizer State with NO culling" << std::endl;


}

EffectVehicle::~EffectVehicle()
{
	// Cleanup DirectX resources (do it in reverse of init)
	using namespace Utils;

	// Texture settings
	SafeRelease(m_pDiffuseMapVar);
	SafeRelease(m_pGlossinessMapVar);
	SafeRelease(m_pSpecularMapVar);
	SafeRelease(m_pNormalMapVar);

	// Scene settings
	SafeRelease(m_pLightDirectionVar);
	SafeRelease(m_pLightIntensityVar);
	SafeRelease(m_pLightColorVar);
	SafeRelease(m_pAmbientLightVar);
	SafeRelease(m_pGlossinessMapVar);

	// Rasterizer Settings
	SafeRelease(m_pRasterizerStateVar);
	SafeRelease(m_pRasterizerStateBFC);
	SafeRelease(m_pRasterizerStateFFC);
	SafeRelease(m_pRasterizerStateNC);

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

void EffectVehicle::SetCullMode(int idx) const
{
	// 0 = Backface culling
	// 1 = Frontface culling
	// 2 = No culling
	
	if(m_pRasterizerStateVar)
	{
		switch(idx)
		{
			case 0:
				m_pRasterizerStateVar->SetRasterizerState(0, m_pRasterizerStateBFC);
				break;
			case 1:
				m_pRasterizerStateVar->SetRasterizerState(0, m_pRasterizerStateFFC);
				break;
			case 2:
				m_pRasterizerStateVar->SetRasterizerState(0, m_pRasterizerStateNC);
				break;
			default:
				m_pRasterizerStateVar->SetRasterizerState(0, m_pRasterizerStateNC);
				assert(false && "Invalid cullmode index given");
				break;
					
		}
	}
}


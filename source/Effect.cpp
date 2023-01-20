#include "pch.h"
#include "Effect.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);
	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");

	if(!m_pTechnique->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}

	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if(!m_pMatWorldViewProjVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldViewProjVariable not valid \n";
	}

	// SAMPLER
	m_pEffectSamplerVariable = m_pEffect->GetVariableByName("gSampler")->AsSampler();
	if(!m_pEffectSamplerVariable->IsValid())
	{
		std::wcout << L"m_pEffectSamplerState is not valid!\n";
	}

	// Matrices and other
	m_pEffectWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if(!m_pEffectWorldMatrixVariable->IsValid())
	{
		std::wcout << L"m_pEffectMatrixVariable is not valid!\n";
	}

	m_pEffectViewInverseMatrixVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if(!m_pEffectViewInverseMatrixVariable->IsValid())
	{
		std::wcout << L"m_pEffectViewInverseMatrixVariable is not valid!\n";
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

	// Point sampler
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	HRESULT hr = pDevice->CreateSamplerState(&samplerDesc, &m_pPointSampler);
	if(FAILED(hr))
		std::wcout << L"Failed to create point sampler state";


	// Linear Sampler
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	hr = pDevice->CreateSamplerState(&samplerDesc, &m_pLinearSampler);
	if(FAILED(hr))
		std::wcout << L"Failed to create linear sampler state";

	// Anisotropic Sampler
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	hr = pDevice->CreateSamplerState(&samplerDesc, &m_pAnisotropicSampler);
	if(FAILED(hr))
		std::wcout << L"Failed to create anisotropic sampler state";
	if(m_pPointSampler && m_pLinearSampler && m_pAnisotropicSampler)
	{
		m_pEffectSamplerVariable->SetSampler(0, m_pPointSampler);
		m_CurrentSamplerFilter = SamplerFilter::Point;
	}
	else
	{
		std::wcout << L"Failed setting sampler\n";
	}
}

Effect::~Effect()
{
	// Delete in reverse order, to prevent issues with dependencies
	using namespace Utils;

	SafeRelease(m_pAnisotropicSampler);
	SafeRelease(m_pLinearSampler);
	SafeRelease(m_pPointSampler);

	SafeRelease(m_pTechnique);
	SafeRelease(m_pEffect);
}

void Effect::SetSamplerFilter(SamplerFilter filter)
{
	switch(filter)
	{
		case Effect::SamplerFilter::Point:
			m_pEffectSamplerVariable->SetSampler(0, m_pPointSampler);
			break;
		case Effect::SamplerFilter::Linear:
			m_pEffectSamplerVariable->SetSampler(0, m_pLinearSampler);
			break;
		case Effect::SamplerFilter::Anisotropic:
			m_pEffectSamplerVariable->SetSampler(0, m_pAnisotropicSampler);
			break;
		default:
			break;
	}
}




/* --------- STATIC FUNCTIONS --------- */

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result{};
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect{};

	DWORD shaderFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if(FAILED(result))
	{
		if(pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for(unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); i++)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}


	return pEffect;
}

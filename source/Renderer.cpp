#include "pch.h"
#include "Renderer.h"
#include "Camera.h"
#include "Texture.h"

#include "EffectVehicle.h"
#include "EffectFire.h"
#include <cassert>
#include "Utils.h"

using Utils::PrintColor;
using Utils::TextColor;

Renderer::Renderer(SDL_Window* pWindow):
	m_pWindow(pWindow),
	m_pCamera{ nullptr },
	m_Scene{}
{
	PrintConsoleCommands();

	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	// Not in the init list, because we need the width and height from previous function
	m_pCamera = new Camera({ 0,0,0 }, 45.0f, 1.0f, 100.0f, m_Width / (float)m_Height);
	m_Scene.AmbientLight = { 0.025f, 0.025f, 0.025f };
	m_Scene.Light = DirectionalLight({ .577f, -.577f, .577f }, 7.f);  // Direction + Intensity



	//Initialize DirectX pipeline
	const HRESULT result = InitializeDirectX();
	if(result == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is initialized and ready!\n";
	}
	else
	{
		std::cout << "DirectX initialization failed!\n";
	}


	// Load in the resources and materials
	m_pVehicleMaterial = new EffectVehicle{ m_pDevice, L"Resources/ShaderFiles/ShaderDefault.fx" };
	m_pFireMaterial = new EffectFire{ m_pDevice, L"Resources/ShaderFiles/ShaderTransparent.fx" };

	m_pVehicleDiffuse = Texture::LoadFromFile(m_pDevice, "./Resources/vehicle_diffuse.png");
	m_pVehicleNormal = Texture::LoadFromFile(m_pDevice, "./Resources/vehicle_normal.png");
	m_pVehicleSpecular = Texture::LoadFromFile(m_pDevice, "./Resources/vehicle_specular.png");
	m_pVehicleGloss = Texture::LoadFromFile(m_pDevice, "./Resources/vehicle_gloss.png");

	m_pFireDiffuse = Texture::LoadFromFile(m_pDevice, "./Resources/fireFX_diffuse.png");

	m_pVehicleMaterial->SetDiffuseMap(m_pVehicleDiffuse);
	m_pVehicleMaterial->SetNormalMap(m_pVehicleNormal);
	m_pVehicleMaterial->SetSpecularMap(m_pVehicleSpecular);
	m_pVehicleMaterial->SetGlossinessMap(m_pVehicleGloss);

	m_pFireMaterial->SetDiffuseMap(m_pFireDiffuse);

	// Load in the meshes
	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};

	Utils::ParseOBJ("./Resources/vehicle.obj", vertices, indices);
	Mesh* pMesh = m_MeshPtrs.emplace_back(new Mesh{ m_pDevice, m_pVehicleMaterial, vertices, indices, {0, 0, 50.0f} });

	Utils::ParseOBJ("./Resources/fireFX.obj", vertices, indices);
	pMesh = m_MeshPtrs.emplace_back(new Mesh{ m_pDevice, m_pFireMaterial, vertices, indices, {0, 0, 50.0f} });

}

Renderer::~Renderer()
{
	using namespace Utils;

	// Deleting Direct X stuff
	SafeRelease(m_pRenderTargetView);
	SafeRelease(m_pRenderTargetBuffer);

	SafeRelease(m_pDepthStencilView);
	SafeRelease(m_pDepthStencilBuffer);

	SafeRelease(m_pSwapChain);

	m_pDeviceContext->ClearState();
	m_pDeviceContext->Flush();
	m_pDeviceContext->Release();

	SafeRelease(m_pDevice);

	// Deleting all the meshes
	// Deleting using std::for_each (mostly a test what I can do with the algorithms from programming 3)
	//std::for_each(begin(m_MeshPtrs), end(m_MeshPtrs), [](Mesh* pMesh){ delete pMesh; });
	for(Mesh* pMesh : m_MeshPtrs)
		delete pMesh;

	m_MeshPtrs.clear();

	// Delete all the textures and materials
	delete m_pVehicleMaterial;
	delete m_pVehicleDiffuse;
	delete m_pVehicleNormal;
	delete m_pVehicleSpecular;
	delete m_pVehicleGloss;

	delete m_pFireMaterial;
	delete m_pFireDiffuse;

	delete m_pCamera;
}

void Renderer::Update(const Timer* pTimer)
{
	m_pCamera->Update(pTimer);


	if(m_RenderSettings.RotateMeshes)
	{
		const float degreesPerSecond{ 45.0f };
		const float yawAngle = (pTimer->GetElapsed() * degreesPerSecond) * TO_RADIANS;

		// Scene update
		for(Mesh* pMesh : m_MeshPtrs)
		{
			pMesh->RotateY(yawAngle);
		}
	}

}


void Renderer::Render() const
{
	ColorRGB clearColor{ .1f, .1f, .1f };  // Uniform clear color -> Dark Gray

	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Hardware)
	{
		if(!m_IsInitialized)
			return;

		// Clear uniform clear color according to setting
		if(m_RenderSettings.UniformClearColor == false)
			clearColor = ColorRGB{ .39f, .59f, .93f }; // Hardware clear color -> Cornflower blue;

		// DirectX
		//1. CLEAR RTV & DSV
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		// 2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		for(Mesh* pMesh : m_MeshPtrs)
		{
			Matrix worldViewProjectionMatrix{ pMesh->GetWorldMatrix() * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix() };
			pMesh->Render(m_pDeviceContext, worldViewProjectionMatrix, m_pCamera->GetInverseViewMatrix());
		}

		// SWAP THE BACKBUFFER / PRESENT
		m_pSwapChain->Present(0, 0);
	}
	else if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Software)
	{
		return;
	}
	else
	{
		assert(false && "No valid rendermethod");
	}
}

void Renderer::ToggleRenderMethod()
{
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Hardware)
	{

		m_RenderSettings.RenderMethod = RenderSettings::RenderMethods::Software;

	}
	else if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Software)
	{
		m_RenderSettings.RenderMethod = RenderSettings::RenderMethods::Hardware;
	}
	else
	{
		assert(false && "No valid rendermethod");
	}
}

void Renderer::ToggleRotation()
{
	m_RenderSettings.RotateMeshes = !m_RenderSettings.RotateMeshes;

	if(m_RenderSettings.RotateMeshes)
		PrintColor("**(SHARED) Vehicle Rotation ON", TextColor::Yellow);
	else
		PrintColor("**(SHARED) Vehicle Rotation OFF", TextColor::Yellow);

}

void Renderer::CycleCullMode()
{
	switch(m_RenderSettings.CullMode)
	{
		case RenderSettings::CullModes::BackFace:
			m_RenderSettings.CullMode = RenderSettings::CullModes::FrontFace;
			PrintColor("**(SHARED) CullMode = Front", TextColor::Yellow);
			break;

		case RenderSettings::CullModes::FrontFace:
			m_RenderSettings.CullMode = RenderSettings::CullModes::None;
			PrintColor("**(SHARED) CullMode = None", TextColor::Yellow);
			break;

		case RenderSettings::CullModes::None:
			m_RenderSettings.CullMode = RenderSettings::CullModes::BackFace;
			PrintColor("**(SHARED) CullMode = Back", TextColor::Yellow);
			break;
	}
}

void Renderer::ToggleUniformClearColor()
{
	m_RenderSettings.UniformClearColor = !m_RenderSettings.UniformClearColor;

	if(m_RenderSettings.UniformClearColor)
		PrintColor("**(SHARED) Uniform ClearColor ON", TextColor::Yellow);
	else
		PrintColor("**(SHARED) Uniform ClearColor OFF", TextColor::Yellow);
}

void Renderer::TogglePrintFPS()
{
	m_RenderSettings.PrintFPS = !m_RenderSettings.PrintFPS;

	if(m_RenderSettings.PrintFPS)
		PrintColor("**(SHARED) Print FPS ON", TextColor::Green);
	else
		PrintColor("**(SHARED) Print FPS OFF", TextColor::Green);
}

void Renderer::ToggleFireFX()
{
	// HARDWARE ONLY
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Hardware)
	{
		m_RenderSettings.ShowFireFX = !m_RenderSettings.ShowFireFX;

		if(m_RenderSettings.ShowFireFX)
			PrintColor("**(HARDWARE) Fire FX ON", TextColor::Green);
		else
			PrintColor("**(HARDWARE) Fire FX OFF", TextColor::Green);
	}

}

void Renderer::ToggleSampleState()
{
	// HARDWARE ONLY
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Hardware)
	{
		switch(m_RenderSettings.SampleState)
		{
			case RenderSettings::SampleStates::Point:
				m_RenderSettings.SampleState = RenderSettings::SampleStates::Linear;
				PrintColor("**(HARDWARE) Sample Filter = Linear", TextColor::Green);
				break;
			case RenderSettings::SampleStates::Linear:
				m_RenderSettings.SampleState = RenderSettings::SampleStates::Anisotropic;
				PrintColor("**(HARDWARE) Sample Filter = Anisotropic", TextColor::Green);
				break;
			case RenderSettings::SampleStates::Anisotropic:
				m_RenderSettings.SampleState = RenderSettings::SampleStates::Point;
				PrintColor("**(HARDWARE) Sample Filter = Point", TextColor::Green);
				break;
		}
	}
}

void Renderer::CycleShadingMode()
{
	// SOFTWARE ONLY
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Software)
	{
		switch(m_RenderSettings.ShadingMode)
		{
			case RenderSettings::ShadingModes::Combined:
				m_RenderSettings.ShadingMode = RenderSettings::ShadingModes::ObservedArea;
				PrintColor("**(SOFTWARE) Shading Mode = OBSERVED_AREA", TextColor::LightMagenta);
				break;
			case RenderSettings::ShadingModes::ObservedArea:
				m_RenderSettings.ShadingMode = RenderSettings::ShadingModes::Diffuse;
				PrintColor("**(SOFTWARE) Shading Mode = DIFFUSE", TextColor::LightMagenta);
				break;
			case RenderSettings::ShadingModes::Diffuse:
				m_RenderSettings.ShadingMode = RenderSettings::ShadingModes::Specular;
				PrintColor("**(SOFTWARE) Shading Mode = SPECULAR", TextColor::LightMagenta);
				break;
			case RenderSettings::ShadingModes::Specular:
				m_RenderSettings.ShadingMode = RenderSettings::ShadingModes::Combined;
				PrintColor("**(SOFTWARE) Shading Mode = COMBINED", TextColor::LightMagenta);
				break;
		}
	}
}

void Renderer::ToggleNormalMap()
{
	// SOFTWARE ONLY
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Software)
	{
		m_RenderSettings.UseNormalMap = !m_RenderSettings.UseNormalMap;

		if(m_RenderSettings.UseNormalMap)
			PrintColor("**(SOFTWARE) Normal Map ON", TextColor::LightMagenta);
		else
			PrintColor("**(SOFTWARE) Normal Map OFF", TextColor::LightMagenta);
	}
}

void Renderer::ToggleDepthBuffer()
{
	// SOFTWARE ONLY
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Software)
	{
		m_RenderSettings.ShowDepthBuffer = !m_RenderSettings.ShowDepthBuffer;

		if(m_RenderSettings.ShowDepthBuffer)
			PrintColor("**(SOFTWARE) DepthBuffer Visualization ON", TextColor::LightMagenta);
		else
			PrintColor("**(SOFTWARE) DepthBuffer Visualization OFF", TextColor::LightMagenta);
	}
}

void Renderer::ToggleBoundingBox()
{
	// SOFTWARE ONLY
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Software)
	{
		m_RenderSettings.ShowBoundingBox = !m_RenderSettings.ShowBoundingBox;

		if(m_RenderSettings.ShowBoundingBox)
			PrintColor("**(SOFTWARE) BoundingBox Visualization ON", TextColor::LightMagenta);
		else
			PrintColor("**(SOFTWARE) BoundingBox Visualization OFF", TextColor::LightMagenta);
	}
}

void Renderer::PrintConsoleCommands()
{
	const TextColor sharedTextColor{ TextColor::Yellow };
	const TextColor hardwareTextColor{ TextColor::Green };
	const TextColor softwareTextColor{ TextColor::LightMagenta };

	PrintColor("[Key Bindings - SHARED]", sharedTextColor);
	PrintColor("    [F1] Toggle Rasterizer Mode (Hardware/Software)", sharedTextColor);
	PrintColor("    [F2] Toggle Vehicle Rotation (ON/OFF)", sharedTextColor);
	PrintColor("    [F9]  Cycle CullMode (BACK/FRONT/NONE)", sharedTextColor);
	PrintColor("    [F10] Toggle Uniform ClearColor (ON/OFF)", sharedTextColor);
	PrintColor("    [F11] Toggle Print FPS (ON/OFF)", sharedTextColor);
	std::cout << std::endl;

	PrintColor("[Key Bindings - HARDWARE]", hardwareTextColor);
	PrintColor("    [F3] Toggle FireFX (ON/OFF)", hardwareTextColor);
	PrintColor("    [F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)", hardwareTextColor);
	std::cout << std::endl;

	PrintColor("[Key Bindings - SOFTWARE]", softwareTextColor);
	PrintColor("    [F5] Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)", softwareTextColor);
	PrintColor("    [F6] Toggle NormalMap (ON/OFF)", softwareTextColor);
	PrintColor("    [F7] Toggle DepthBuffer Visualization (ON/OFF)", softwareTextColor);
	PrintColor("    [F8] Toggle BoundingBox Visualization (ON/OFF)", softwareTextColor);
	std::cout << std::endl;

}

HRESULT Renderer::InitializeDirectX()
{
	// 1. Create device & device context
	// ==============================

	D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
	uint32_t createDeviceFlags{ 0 };

#if defined(debug) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result{ D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		createDeviceFlags,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&m_pDevice,
		nullptr,
		&m_pDeviceContext
		) };

	if(FAILED(result))
	{
		std::cout << "Error creating device\n";
		return result;
	}


	// Create DXGI Factory
	IDXGIFactory1* pDxgiFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));

	if(FAILED(result))
	{
		std::cout << "Error creating factory\n";
		return result;
	}


	// 2. Create Swapchain
	// ==============================

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;


	// Get the handle (HWND) from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	// Create the swapchain using the given settings
	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	pDxgiFactory->Release();

	if(FAILED(result))
	{
		std::cout << "Error getting SDL window\n";
		return result;
	}


	// 3. Create the DepthStencis (DS) & DepthStencilView (DSV)
	// ==============================
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if(FAILED(result))
	{
		std::cout << "Error creating depth stencil\n";
		return result;
	}

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if(FAILED(result))
	{
		std::cout << "Error creating depth stencil view\n";
		return result;
	}

	// 4. Create RenderTraget (RT) & RenderTargetView (RTV)
	// ==============================

	// Resource
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if(FAILED(result))
	{
		std::cout << "Error creating render target buffer\n";
		return result;
	}

	// View
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if(FAILED(result))
	{
		std::cout << "Error creating render target view\n";
		return result;
	}

	// 5. Bind RTV & DSV to Output Merger Stage
	// ==============================
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// 6. Set the Viewport
	// ==============================
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);



	return result;
}

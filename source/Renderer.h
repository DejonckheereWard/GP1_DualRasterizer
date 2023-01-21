#pragma once
#include "Effect.h"

struct SDL_Window;
struct SDL_Surface;
class Camera;

class EffectVehicle;
class EffectFire;

using namespace dae;

// Extra structs for the scene
struct DirectionalLight
{
	Vector3 Direction{ .577f, -.577f, .577f};
	float Intensity{ 7.f };
	ColorRGB Color{ 1.0f, 1.0f, 1.0f };
};

struct SceneSettings
{
	DirectionalLight Light{};
	ColorRGB AmbientLight{ 0.025f, 0.025f , 0.025f};
	float Shininess{ 25.0f };
};


struct RenderSettings
{
	enum class RenderMethods
	{
		Hardware,
		Software
	};

	enum class CullModes
	{
		BackFace,
		FrontFace,
		None,
	};

	enum class SampleStates
	{
		Point,
		Linear,
		Anisotropic
	};

	enum class ShadingModes
	{
		Combined,
		ObservedArea,
		Diffuse,		// Includes ObservedArea
		Specular		// Includes ObservedArea
	};

	// Shared
	RenderMethods RenderMethod = RenderMethods::Hardware;
	bool RotateMeshes = true;
	CullModes CullMode = CullModes::BackFace;
	bool UniformClearColor = false;

	// Hardware only
	bool ShowFireFX = true;
	Effect::SamplerFilter SampleState = Effect::SamplerFilter::Point;
	
	// Software only
	ShadingModes ShadingMode = ShadingModes::Combined;
	bool UseNormalMap = true;
	bool ShowDepthBuffer = false;
	bool ShowBoundingBox = false;
	
};

class Renderer final
{
public:
	enum class FilterMethod
	{
		Point,
		Linear,
		Anisotropic
	};

	Renderer(SDL_Window* pWindow);

	// Rule of 5
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) noexcept = delete;
	
	void Update(const Timer* pTimer);
	void Render() const;
	void RenderSoftware() const;
	void RenderHardware() const;

	// Shared
	void ToggleRenderMethod();
	void ToggleRotation();
	void CycleCullMode();
	void ToggleUniformClearColor();
	
	// Hardware
	void ToggleFireFX();
	void ToggleSampleFilter();

	// Software
	void CycleShadingMode();
	void ToggleNormalMap();
	void ToggleDepthBuffer();
	void ToggleBoundingBox();

private:
	SDL_Window* m_pWindow{};

	int m_Width{};
	int m_Height{};



	bool m_IsInitialized{ false };
	void PrintConsoleCommands();

	// Shared -----------------------------
	Camera* m_pCamera;  // Unique pointer for camera (could make it shared if needed)
	std::vector<Mesh*> m_MeshPtrs;
	
	RenderSettings m_RenderSettings{};
	SceneSettings m_SceneSettings;
	const ColorRGB m_UniformClearColor{ 0.1f, 0.1f, 0.1f };  // -> Dark Gray


	// Textures
	Texture* m_pVehicleDiffuse{};
	Texture* m_pVehicleNormal{};
	Texture* m_pVehicleSpecular{};
	Texture* m_pVehicleGloss{};
	Texture* m_pFireDiffuse{};


	// Software ----------------------------
	void VertexTransformationFunction(const std::vector<Mesh*>& meshes) const;
	ColorRGB PixelShader(const Vertex_Out& vert) const;  // Software pixel shader
	SDL_Surface* m_pFrontBuffer{ nullptr };
	SDL_Surface* m_pBackBuffer{ nullptr };
	uint32_t* m_pBackBufferPixels{};
	float* m_pDepthBufferPixels{};

	// DIRECTX -----------------------------
	HRESULT InitializeDirectX();

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

	IDXGISwapChain* m_pSwapChain;

	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilView* m_pDepthStencilView;

	ID3D11Texture2D* m_pRenderTargetBuffer;
	ID3D11RenderTargetView* m_pRenderTargetView;


	EffectVehicle* m_pVehicleMaterial;
	EffectFire* m_pFireMaterial;
};

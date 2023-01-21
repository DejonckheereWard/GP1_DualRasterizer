#pragma once

struct SDL_Window;
struct SDL_Surface;
class Camera;

class EffectVehicle;
class EffectFire;

using namespace dae;

// Extra structs for the scene
struct DirectionalLight
{
	Vector3 Direction;
	float Intensity;
	ColorRGB Color;
};

struct SceneSettings
{
	DirectionalLight Light;
	ColorRGB AmbientLight;
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
	bool PrintFPS = false;

	// Hardware only
	bool ShowFireFX = true;
	SampleStates SampleState = SampleStates::Point;
	
	// Software only
	ShadingModes ShadingMode = ShadingModes::Combined;
	bool UseNormalMap = false;
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

	// Shared
	void ToggleRenderMethod();
	void ToggleRotation();
	void CycleCullMode();
	void ToggleUniformClearColor();
	void TogglePrintFPS();
	
	// Hardware
	void ToggleFireFX();
	void ToggleSampleState();

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
	SceneSettings m_Scene;

	// Textures
	Texture* m_pVehicleDiffuse{};
	Texture* m_pVehicleNormal{};
	Texture* m_pVehicleSpecular{};
	Texture* m_pVehicleGloss{};
	Texture* m_pFireDiffuse{};


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

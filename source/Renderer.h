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

class Renderer final
{
public:
	enum class FilterMethod
	{
		Point,
		Linear,
		Anisotropic
	};

	enum class RenderMethod
	{
		Hardware,
		Software
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

	void ToggleRenderMethod() { m_RenderMethod = m_RenderMethod == RenderMethod::Hardware ? RenderMethod::Software : RenderMethod::Hardware; }

private:
	SDL_Window* m_pWindow{};

	int m_Width{};
	int m_Height{};



	bool m_IsInitialized{ false };

	// Shared -----------------------------
	Camera* m_pCamera;  // Unique pointer for camera (could make it shared if needed)
	std::vector<Mesh*> m_MeshPtrs;
	SceneSettings m_Scene;

	RenderMethod m_RenderMethod{ RenderMethod::Hardware };

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

#pragma once

struct SDL_Window;
struct SDL_Surface;
class Camera;

using namespace dae;

class Renderer final
{
public:
	Renderer(SDL_Window* pWindow);

	// Rule of 5
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) noexcept = delete;

	void Update(const Timer* pTimer);
	void Render() const;

private:
	SDL_Window* m_pWindow{};

	int m_Width{};
	int m_Height{};

	

	bool m_IsInitialized{ false };

	Camera* m_pCamera;  // Unique pointer for camera (could make it shared if needed)
	


	//DIRECTX
	HRESULT InitializeDirectX();
	//...
};

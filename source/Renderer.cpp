#include "pch.h"
#include "Renderer.h"
#include "Camera.h"
//#include "Mesh.h";
//#include "Utils.h"


Renderer::Renderer(SDL_Window* pWindow):
	m_pWindow(pWindow),
	m_pCamera{ std::make_unique<Camera>(Camera({0,0,0}, 45.0f, 1.0f, 100.0f)) }
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);


	


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
}

Renderer::~Renderer()
{

}

void Renderer::Update(const Timer* pTimer)
{

}


void Renderer::Render() const
{
	if(!m_IsInitialized)
		return;

}

HRESULT Renderer::InitializeDirectX()
{
	return S_FALSE;
}

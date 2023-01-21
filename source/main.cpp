#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

bool gPrintFPS{ false };

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void TogglePrintFPS()
{
	gPrintFPS = !gPrintFPS;

	if(gPrintFPS)
		Utils::PrintColor("**(SHARED) Print FPS ON", Utils::TextColor::Green);
	else
		Utils::PrintColor("**(SHARED) Print FPS OFF", Utils::TextColor::Green);
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DualRasterizer - Dejonckheere Ward (2DAE07)",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if(!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	while(isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
				case SDL_QUIT:
					isLooping = false;
					break;
				case SDL_KEYUP:
					switch(e.key.keysym.scancode)
					{
						// Shared --------------------------------------------------------------
						case SDL_SCANCODE_F1:
							pRenderer->ToggleRenderMethod();
							break;
						case SDL_SCANCODE_F2:
							pRenderer->ToggleRotation();
							break;

						case SDL_SCANCODE_F9:
							pRenderer->CycleCullMode();
							break;

						case SDL_SCANCODE_F10:
							pRenderer->ToggleUniformClearColor();
							break;

						case SDL_SCANCODE_F11:
							TogglePrintFPS();
							break;

							// Hardware rasterizer only ----------------------------------------
						case SDL_SCANCODE_F3:
							pRenderer->ToggleFireFX();
							break;
						case SDL_SCANCODE_F4:
							pRenderer->ToggleSampleFilter();
							break;

							// Software rasterizer only ----------------------------------------
						case SDL_SCANCODE_F5:
							pRenderer->CycleShadingMode();
							break;

						case SDL_SCANCODE_F6:
							pRenderer->ToggleNormalMap();
							break;
						case SDL_SCANCODE_F7:
							pRenderer->ToggleDepthBuffer();
							break;
						case SDL_SCANCODE_F8:
							pRenderer->ToggleBoundingBox();
							break;
					}
					break;
				default:;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if(printTimer >= 1.f)
		{
			printTimer = 0.f;
			if(gPrintFPS)
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}
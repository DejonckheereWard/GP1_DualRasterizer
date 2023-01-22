#include "pch.h"
#include "Renderer.h"
#include "Camera.h"
#include "Texture.h"

#include "EffectVehicle.h"
#include "EffectFire.h"
#include <cassert>
#include "Utils.h"

#include <ppl.h>

using Utils::PrintColor;
using Utils::TextColor;

Renderer::Renderer(SDL_Window* pWindow):
	m_pWindow(pWindow),
	m_pCamera{ nullptr },
	m_SceneSettings{}
{
	PrintConsoleCommands();
	PrintExtraInfo();

	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	// Not in the init list, because we need the width and height from previous function
	m_pCamera = new Camera({ 0,0,0 }, 45.0f, 1.0f, 100.0f, m_Width / (float)m_Height);


	// Init Software Rasterizer ----------------------------
	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_pDepthBufferPixels = new float[m_Width * m_Height];

	// Init Hardware Rasterizer ----------------------------
	//Initialize DirectX pipeline
	const HRESULT result = InitializeDirectX();
	if(result == S_OK)
	{
		m_IsInitialized = true;
		//std::cout << "DirectX is initialized and ready!\n";
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


	// Set the scene settings for the all supported meshes
	for(Mesh* pMesh : m_MeshPtrs)
	{
		Effect* pEffect{ pMesh->GetEffect() };

		// Try casting to EffectVehicle
		EffectVehicle* pEffectVehicle{ dynamic_cast<EffectVehicle*>(pEffect) };
		if(pEffectVehicle)
		{
			pEffectVehicle->SetLightDirection(m_SceneSettings.Light.Direction);
			pEffectVehicle->SetLightIntensity(m_SceneSettings.Light.Intensity);
			pEffectVehicle->SetLightColor(m_SceneSettings.Light.Color);
			pEffectVehicle->SetAmbientlight(m_SceneSettings.AmbientLight);
			pEffectVehicle->SetShininess(m_SceneSettings.Shininess);
			
			// Made sure the cullmodes and the indexes matched 1 to 1, i didnt want cullmodes to be defined in 2 places;
			pEffectVehicle->SetCullMode(int(m_RenderSettings.CullMode));
		}
	}

}

Renderer::~Renderer()
{
	using namespace Utils;

	// Deleting software stuff
	SDL_FreeSurface(m_pBackBuffer);
	SDL_FreeSurface(m_pFrontBuffer);
	delete[] m_pDepthBufferPixels;


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

	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Hardware)
	{
		m_MeshPtrs.at(1)->SetVisibility(m_RenderSettings.ShowFireFX);

		RenderHardware();
	}
	else if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Software)
	{
		m_MeshPtrs.at(1)->SetVisibility(false);

		//@START
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);

		// Execute Software Rasterizer
		RenderSoftware();

		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}
}

void Renderer::RenderSoftware() const
{
	ColorRGB clearColor{ m_UniformClearColor };

	// Clear uniform clear color according to setting
	if(m_RenderSettings.UniformClearColor == false)
		clearColor = ColorRGB{ .39f, .39f, .39f }; // Software clear color -> Light gray;

	// Software raytracer takes the color in range 0-255 and not as floats
	clearColor *= 255.0f;

	// Clear back buffer
	uint32_t hexColor = 0xFF000000 | (uint32_t)clearColor.b << 16 | (uint32_t)clearColor.g << 8 | (uint32_t)clearColor.r;
	SDL_FillRect(m_pBackBuffer, NULL, hexColor);

	// Clear depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	VertexTransformationFunction(m_MeshPtrs);

	for(const Mesh* pMesh : m_MeshPtrs)
	{
		if(!pMesh->Visible())
			continue;

		//VertexTransformationFunction(mesh.vertices, mesh_screen.vertices);

		// If triangle strip, move only one position per itteration & inverse the direction on every odd loop
		int increment = 3;
		if(pMesh->GetTopology() == PrimitiveTopology::TriangleStrip)
			increment = 1;


		concurrency::parallel_for(0u, uint32_t((pMesh->indices.size() - 2) / increment), [=, this](uint32_t index)
		{
			{
				uint32_t indiceIdx{ index * increment };
				// Get the vertices using the indice numbers
				uint32_t indiceA{ pMesh->indices[indiceIdx] };
				uint32_t indiceB{ pMesh->indices[indiceIdx + 1] };
				uint32_t indiceC{ pMesh->indices[indiceIdx + 2] };

				Vertex_Out A{ pMesh->vertices_out[indiceA] };
				Vertex_Out B{ pMesh->vertices_out[indiceB] };
				Vertex_Out C{ pMesh->vertices_out[indiceC] };

				// If triangle strip, move only one position per itteration & inverse the direction on every odd loop

				if(pMesh->GetTopology() == PrimitiveTopology::TriangleStrip)
				{
					// Check if least significant bit is 1 (odd number)
					if((indiceIdx & 1) == 1)
						std::swap(B, C);

					// Check if any vertices of the triangle are the same (and thus the triangle has 0 area / should not be rendered)
					if(indiceA == indiceB)
						return;  // Return instead of continue in parallel fors

					if(indiceB == indiceC)
						return;  // Return instead of continue in parallel fors

					if(indiceC == indiceA)
						return;  // Return instead of continue in parallel fors

				}


				// Do frustum culling
				if(A.position.z < 0.0f || A.position.z > 1.0f)
					return;  // Return instead of continue in parallel fors
				if(B.position.z < 0.0f || B.position.z > 1.0f)
					return;  // Return instead of continue in parallel fors
				if(C.position.z < 0.0f || C.position.z > 1.0f)
					return;  // Return instead of continue in parallel fors

				if(A.position.x < -1.0f || A.position.x > 1.0f)
					if(B.position.x < -1.0f || B.position.x > 1.0f)
						if(C.position.x < -1.0f || C.position.x > 1.0f)
							return;  // Return instead of continue in parallel fors

				if(A.position.y < -1.0f || A.position.y > 1.0f)
					if(B.position.y < -1.0f || B.position.y > 1.0f)
						if(C.position.y < -1.0f || C.position.y > 1.0f)
							return;  // Return instead of continue in parallel fors


				// Convert from NDC to ScreenSpace
				A.position.x = (A.position.x + 1) / 2.0f * m_Width; // Screen X
				A.position.y = (1 - A.position.y) / 2.0f * m_Height; // Screen Y,
				B.position.x = (B.position.x + 1) / 2.0f * m_Width; // Screen X
				B.position.y = (1 - B.position.y) / 2.0f * m_Height; // Screen Y,
				C.position.x = (C.position.x + 1) / 2.0f * m_Width; // Screen X
				C.position.y = (1 - C.position.y) / 2.0f * m_Height; // Screen Y,

				// Define the edges of the screen triangle
				const Vector2 edgeA{ A.position.GetXY(), B.position.GetXY() };
				const Vector2 edgeB{ B.position.GetXY(), C.position.GetXY() };
				const Vector2 edgeC{ C.position.GetXY(), A.position.GetXY() };


				// Get the bounding box of the triangle (min max)
				Vector2 bbMin;
				bbMin.x = std::min(A.position.x, std::min(B.position.x, C.position.x));
				bbMin.y = std::min(A.position.y, std::min(B.position.y, C.position.y));

				Vector2 bbMax;
				bbMax.x = std::max(A.position.x, std::max(B.position.x, C.position.x));
				bbMax.y = std::max(A.position.y, std::max(B.position.y, C.position.y));

				bbMin.x = Clamp(bbMin.x, 0.0f, float(m_Width));
				bbMin.y = Clamp(bbMin.y, 0.0f, float(m_Height));

				bbMax.x = Clamp(bbMax.x, 0.0f, float(m_Width));
				bbMax.y = Clamp(bbMax.y, 0.0f, float(m_Height));




				for(int py = int(bbMin.y); py < int(ceil(bbMax.y)); ++py)
				{
					for(int px = int(bbMin.x); px < int(ceil(bbMax.x)); ++px)
					{

						if(m_RenderSettings.ShowBoundingBox)
						{
							// Render white pixels where bounding box is
							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(255),
								static_cast<uint8_t>(255),
								static_cast<uint8_t>(255));
							continue;
						}

						// Get the current pixel into a vector
						Vector2 pixel{ float(px) + 0.5f, float(py) + 0.5f };  // Define pixel as 2D point (take center of the pixel)

						// Get the signed areas of every edge (no division by 2 because triangle area isn't either, and we are only interested in percentage)
						const float signedAreaParallelogramAB{ Vector2::Cross(edgeA, Vector2{ A.position.GetXY(), pixel }) };
						const float signedAreaParallelogramBC{ Vector2::Cross(edgeB, Vector2{ B.position.GetXY(), pixel }) };
						const float signedAreaParallelogramCA{ Vector2::Cross(edgeC, Vector2{ C.position.GetXY(), pixel }) };

						// isInside will turn false if any of the below 3 caclulations returns a negative number (true &= true -> true while true &= false -> false)
						bool isInside = true;

						switch(m_RenderSettings.CullMode)
						{
							case RenderSettings::CullModes::BackFace:
								isInside = signedAreaParallelogramAB >= 0.0f && signedAreaParallelogramBC >= 0.0f && signedAreaParallelogramCA >= 0.0f;
								break;
							case RenderSettings::CullModes::FrontFace:
								isInside = signedAreaParallelogramAB <= 0.0f && signedAreaParallelogramBC <= 0.0f && signedAreaParallelogramCA <= 0.0f;
								break;
							case RenderSettings::CullModes::None:
								isInside = signedAreaParallelogramAB >= 0.0f && signedAreaParallelogramBC >= 0.0f && signedAreaParallelogramCA >= 0.0f;  // Inside Back
								// inside triangle either front or back (|= "or's" the 2 together)
								isInside |= signedAreaParallelogramAB <= 0.0f && signedAreaParallelogramBC <= 0.0f && signedAreaParallelogramCA <= 0.0f; // Inside Front
								break;
							default:
								assert(false);
						}


						if(isInside)
						{
							// Perform clipping
							//if (A.position.x < -1.0f || A.position.x > 1.0f)
							//	continue;

							//if (A.position.y < -1.0f || A.position.y > 1.0f)
							//	continue;


							// Get the weights of each vertex
							const float triangleArea = Vector2::Cross(edgeA, -edgeC);
							const float weightA{ signedAreaParallelogramBC / triangleArea };
							const float weightB{ signedAreaParallelogramCA / triangleArea };
							const float weightC{ signedAreaParallelogramAB / triangleArea };

							// Check if total weight is +/- 1.0f;
							assert((weightA + weightB + weightC) > 0.99f);
							assert((weightA + weightB + weightC) < 1.01f);

							// Get the interpolated Z buffer value
							float zBuffer = 1.0f / ((weightA / A.position.z) + (weightB / B.position.z) + (weightC / C.position.z));

							// Check the depth buffer
							if(zBuffer > m_pDepthBufferPixels[px + (py * m_Width)])
								continue;

							if(zBuffer < 0.0f || zBuffer > 1.0f)
								continue;

							m_pDepthBufferPixels[px + (py * m_Width)] = zBuffer;

							float wInterpolated = 1.0f /
								((1.0f / A.position.w) * weightA + (1.0f / B.position.w) * weightB + (1.0f / C.position.w) * weightC);

							// Get the interpolated UV
							Vector2 uvInterpolated{
								(A.uv / A.position.w) * weightA +
								(B.uv / B.position.w) * weightB +
								(C.uv / C.position.w) * weightC
							};
							uvInterpolated *= wInterpolated;

							// Get the interpolated color
							ColorRGB colorInterpolated{
								(A.color / A.position.w) * weightA +
								(B.color / B.position.w) * weightB +
								(C.color / C.position.w) * weightC
							};
							colorInterpolated *= wInterpolated;

							// Get the interpolated normal
							Vector3 normalInterpolated{
								(A.normal / A.position.w) * weightA +
								(B.normal / B.position.w) * weightB +
								(C.normal / C.position.w) * weightC
							};
							normalInterpolated *= wInterpolated;
							normalInterpolated.Normalize();

							// Get the interpolated tangent
							Vector3 tangentInterpolated{
								(A.tangent / A.position.w) * weightA +
								(B.tangent / B.position.w) * weightB +
								(C.tangent / C.position.w) * weightC
							};
							tangentInterpolated *= wInterpolated;
							tangentInterpolated.Normalize();

							// Get the interpolated viewdirection
							Vector3 viewDirectionInterpolated{
								(A.viewDirection / A.position.w) * weightA +
								(B.viewDirection / B.position.w) * weightB +
								(C.viewDirection / C.position.w) * weightC
							};
							viewDirectionInterpolated *= wInterpolated;
							viewDirectionInterpolated.Normalize();


							Vertex_Out vertexOut{};
							vertexOut.position = Vector4{ pixel.x, pixel.y, zBuffer, wInterpolated };
							vertexOut.color = colorInterpolated;
							vertexOut.uv = uvInterpolated;
							vertexOut.normal = normalInterpolated;
							vertexOut.tangent = tangentInterpolated;
							vertexOut.viewDirection = viewDirectionInterpolated;

							//PixelShading(vertexOut);

							ColorRGB finalColor{};
							if(m_RenderSettings.ShowDepthBuffer)
							{
								const float remapMin{ 0.970f };
								const float remapMax{ 1.0f };

								float depthColor = (Clamp(zBuffer, remapMin, remapMax) - remapMin) / (remapMax - remapMin);

								finalColor = { depthColor, depthColor, depthColor };
							}
							else
							{
								finalColor = PixelShader(vertexOut);
							}


							finalColor.MaxToOne();
							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(finalColor.r * 255),
								static_cast<uint8_t>(finalColor.g * 255),
								static_cast<uint8_t>(finalColor.b * 255));

						}
					}
				}
			}
		});
	}

}

void Renderer::RenderHardware() const
{
	ColorRGB clearColor{ m_UniformClearColor };

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
		if(!pMesh->Visible())
			continue;

		Matrix worldViewProjectionMatrix{ pMesh->GetWorldMatrix() * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix() };
		pMesh->Render(m_pDeviceContext, worldViewProjectionMatrix, m_pCamera->GetInverseViewMatrix());
	}

	// SWAP THE BACKBUFFER / PRESENT
	m_pSwapChain->Present(0, 0);
}

void Renderer::ToggleRenderMethod()
{
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Hardware)
	{
		PrintColor("**(SHARED)Rasterizer Mode = SOFTWARE", TextColor::Yellow);
		m_RenderSettings.RenderMethod = RenderSettings::RenderMethods::Software;

	}
	else if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Software)
	{
		PrintColor("**(SHARED)Rasterizer Mode = HARDWARE", TextColor::Yellow);
		m_RenderSettings.RenderMethod = RenderSettings::RenderMethods::Hardware;
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

	SetShaderCullModes();
}

void Renderer::ToggleUniformClearColor()
{
	m_RenderSettings.UniformClearColor = !m_RenderSettings.UniformClearColor;

	if(m_RenderSettings.UniformClearColor)
		PrintColor("**(SHARED) Uniform ClearColor ON", TextColor::Yellow);
	else
		PrintColor("**(SHARED) Uniform ClearColor OFF", TextColor::Yellow);
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

void Renderer::ToggleSampleFilter()
{
	// HARDWARE ONLY
	if(m_RenderSettings.RenderMethod == RenderSettings::RenderMethods::Hardware)
	{
		switch(m_RenderSettings.SampleState)
		{
			case Effect::SamplerFilter::Point:
				m_RenderSettings.SampleState = Effect::SamplerFilter::Linear;
				PrintColor("**(HARDWARE) Sample Filter = Linear", TextColor::Green);
				break;

			case Effect::SamplerFilter::Linear:
				m_RenderSettings.SampleState = Effect::SamplerFilter::Anisotropic;
				PrintColor("**(HARDWARE) Sample Filter = Anisotropic", TextColor::Green);
				break;

			case Effect::SamplerFilter::Anisotropic:
				m_RenderSettings.SampleState = Effect::SamplerFilter::Point;
				PrintColor("**(HARDWARE) Sample Filter = Point", TextColor::Green);
				break;
		}

		// loop over every mesh and set the effect
		for(Mesh* pMesh : m_MeshPtrs)
		{
			pMesh->GetEffect()->SetSamplerFilter(m_RenderSettings.SampleState);
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

void Renderer::PrintExtraInfo()
{
	PrintColor("[Extra Features]", TextColor::LightCyan);
	PrintColor("    Multithreading for the Software Rasterizer (VertexTransformation and Render loop)", TextColor::LightCyan);
	std::cout << std::endl;

}

void Renderer::VertexTransformationFunction(const std::vector<Mesh*>& meshes) const
{
	// This upper multithreading loop might make more impact if there were more meshes, but since  i dont render the fire
	// Theres only really one mesh to render anyway, making this kinda redundant in this scenario. but im leaving this in either way.
	concurrency::parallel_for(0u, (uint32_t)meshes.size(), [=, this](uint32_t index)
	{
		{
			Mesh* pMesh = meshes[index];
			// Calculate WorldViewProjectionmatrix for every mesh	
			Matrix meshWorldMatrix{ pMesh->GetWorldMatrix() };
			Matrix worldViewProjectionMatrix = meshWorldMatrix * (m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix());

			pMesh->vertices_out.clear();
			pMesh->vertices_out.reserve(pMesh->vertices.size());

			// For the parallelization, i wanted existing slots to fill in the out vertices, hen
			// Using pushback or emplace back made the order of vertices all messed up (and ended up breaking the 3D model)
			pMesh->vertices_out.resize(pMesh->vertices.size());

			// Multithread the vertex loop
			concurrency::parallel_for(0u, (uint32_t)pMesh->vertices.size(), [=, this](uint32_t index)
			{
				{
					const Vertex& vert{ pMesh->vertices[index] };

					// World to camera (view space)
					Vector4 newPosition = worldViewProjectionMatrix.TransformPoint({ vert.position, 1.0f });

					// Perspective divide 
					newPosition.x /= newPosition.w;
					newPosition.y /= newPosition.w;
					newPosition.z /= newPosition.w;

					// Our coords are now in NDC space
					// Multiply the normals and tangents with the worldmatrix to convert them to worldspace
					 //We only want to rotate them, so use transformvector, and normalize after
					const Vector3 newNormal = meshWorldMatrix.TransformVector(vert.normal).Normalized();
					const Vector3 newTangent = meshWorldMatrix.TransformVector(vert.tangent).Normalized();

					// Calculate vert world position
					const Vector3 vertPosition{ pMesh->GetWorldMatrix().TransformPoint(vert.position) };

					// Store the new position in the vertices out as Vertex out, because this one has a position 4 / vector4
					Vertex_Out& outVert = pMesh->vertices_out[index];
					outVert.position = newPosition;
					outVert.color = vert.color;
					outVert.uv = vert.uv;
					outVert.normal = newNormal;
					outVert.tangent = newTangent;
					outVert.viewDirection = { vertPosition - m_pCamera->GetOrigin() };
				}
			});
		}
	});
}

ColorRGB Renderer::PixelShader(const Vertex_Out& vert) const
{
	const Vector3 lightDirection{ m_SceneSettings.Light.Direction };
	const ColorRGB lightColor{ m_SceneSettings.Light.Color };
	const float lightIntensity{ m_SceneSettings.Light.Intensity };
	const ColorRGB ambientColor{ m_SceneSettings.AmbientLight };
	const float specularGlossiness{ m_SceneSettings.Shininess };  // Shininess

	const ColorRGB lightRadiance{ lightColor * lightIntensity };

	const ColorRGB diffuseColorSample{ m_pVehicleDiffuse->Sample(vert.uv) };
	const ColorRGB specularColorSample{ m_pVehicleSpecular->Sample(vert.uv) };
	const ColorRGB normalColorSample{ m_pVehicleNormal->Sample(vert.uv) };
	const ColorRGB glossinessColor{ m_pVehicleGloss->Sample(vert.uv) };


	//// Calculate tangent space axis
	const Vector3 binormal{ Vector3::Cross(vert.normal, vert.tangent) };
	const Matrix tangentSpaceAxis{ Matrix{ vert.tangent, binormal, vert.normal, Vector3::Zero } };  // {} = 0 vector


	ColorRGB x = 2.0f * normalColorSample - colors::White;
	////Calculate normal in tangent space
	const Vector3 tangentNormal{ normalColorSample.r * 2.0f - 1.0f, normalColorSample.g * 2.0f - 1.0f, normalColorSample.b * 2.0f - 1.0f };
	const Vector3 normalInTangentSpace{ tangentSpaceAxis.TransformVector(tangentNormal.Normalized()).Normalized() };

	//// Select normal based on settings
	const Vector3 currentNormal{ m_RenderSettings.UseNormalMap ? normalInTangentSpace : vert.normal };

	//// Calculate observed area / lambert Cosine
	const float observedArea{ Vector3::Dot(currentNormal, -lightDirection) };

	if(observedArea < 0.0f)
		return { 0,0,0 };


	// Calculate lambert
	const ColorRGB lambertDiffuse{ (1.0f * diffuseColorSample) / PI };

	// Calculate phong
	const Vector3 reflect{ lightDirection - (2.0f * Vector3::Dot(currentNormal, lightDirection) * currentNormal) };
	const float RdotV{ std::max(0.0f, Vector3::Dot(reflect, -vert.viewDirection)) };
	const ColorRGB phongSpecular{ specularColorSample * powf(RdotV, glossinessColor.r * specularGlossiness) }; // Glosinness map is greyscale, ro r g and b are the same

	switch(m_RenderSettings.ShadingMode)
	{
		case RenderSettings::ShadingModes::Combined:
			return ((lightRadiance * lambertDiffuse) + phongSpecular + ambientColor) * observedArea;
			break;
		case RenderSettings::ShadingModes::ObservedArea:
			return ColorRGB{ observedArea, observedArea, observedArea };
			break;
		case RenderSettings::ShadingModes::Diffuse:
			return lightRadiance * lambertDiffuse * observedArea;
			break;
		case RenderSettings::ShadingModes::Specular:
			return phongSpecular;
			break;
		default:
			return { 0,0,0 };
			break;
	}
}


void Renderer::SetShaderCullModes()
{
	for(Mesh* pMesh : m_MeshPtrs)
	{
		Effect* pEffect{ pMesh->GetEffect() };

		// Try casting to EffectVehicle, only set cullmode if cast worked
		// Dynamic casts not ideal
		EffectVehicle* pEffectVehicle{ dynamic_cast<EffectVehicle*>(pEffect) };
		if(pEffectVehicle)
		{
			// I made sure the cullmode and index matched up, i didnt want cullmodes to be defined in 2 spots, hence the conversion
			pEffectVehicle->SetCullMode(int(m_RenderSettings.CullMode));
		}
	}
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

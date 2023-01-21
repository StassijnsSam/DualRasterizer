#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "RenderManager.h"
#include "Timer.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
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
		"DualRasterizer Stassijns Sam 2DAE08",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new dae::Timer();
	const auto pRenderManager = new RenderManager(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				if (e.key.keysym.scancode == SDL_SCANCODE_F1) {
					//Toggle between DirectX and Software
					pRenderManager->ToggleRenderType();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F2) {
					//Toggle rotation
					pRenderManager->ToggleRotation();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F3) {
					//FireMesh off in hardware
					pRenderManager->ToggleFire();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4) {
					//Cycle hardware sampler state
					pRenderManager->CycleSamplerState();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F5) {
					//Cycle software shading mode
					pRenderManager->CycleShadingMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F6) {
					//Toggle software normal map
					pRenderManager->ToggleNormalMap();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F7) {
					//Toggle software depth buffer
					pRenderManager->ToggleDepthBuffer();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F8) {
					//Toggle software bounding box
					pRenderManager->ToggleBoundingBox();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F9) {
					//Cycle cull mode
					pRenderManager->CycleCullMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F10) {
					//Toggle clearColor
					pRenderManager->ToggleClearColor();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F11) {
					//Toggle print FPW
					pRenderManager->TogglePrintFPW();
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderManager->Update(pTimer);

		//--------- Render ---------
		pRenderManager->Render();

		//--------- Timer ---------
		pTimer->Update();
		if (pRenderManager->CanPrintFPW()) {
			printTimer += pTimer->GetElapsed();
			if (printTimer >= 1.f)
			{
				printTimer = 0.f;
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}	
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderManager;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}
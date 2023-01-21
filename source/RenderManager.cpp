#include "pch.h"
#include "RenderManager.h"

namespace dae {

	RenderManager::RenderManager(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
	}

	RenderManager::~RenderManager()
	{
		
	}

	void RenderManager::Update(const Timer* pTimer)
	{

	}


	void RenderManager::Render() const
	{
		if (!m_IsInitialized)
			return;

	}

	HRESULT RenderManager::InitializeDirectX()
	{
		return S_FALSE;
	}
}

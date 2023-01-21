#include "pch.h"
#include "Renderer.h"
#include "DataTypes.h"

Renderer::Renderer(SDL_Window* pWindow, dae::Camera* pCamera, std::vector<Mesh*> pMeshes) :
	m_pWindow{ pWindow },
	m_pCamera{ pCamera },
	m_pMeshes{pMeshes}
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	//Initialize Camera
	//m_Camera.Initialize(60.f, { .0f,5.f, -30.f }, m_AspectRatio);
}

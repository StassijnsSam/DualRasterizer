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
}

void Renderer::ToggleRotation()
{
	m_CanRotate = !m_CanRotate;
}

void Renderer::ToggleClearColor()
{
	m_ShouldUseUniformColor = !m_ShouldUseUniformColor;
}

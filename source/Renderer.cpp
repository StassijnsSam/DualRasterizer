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

void Renderer::CycleCullmode()
{
	switch (m_CurrentCullmode) {
	case Cullmode::backFace:
		m_CurrentCullmode = Cullmode::frontFace;
		std::cout << "Cullmode changed to front face culling" << std::endl;
		break;
	case Cullmode::frontFace:
		m_CurrentCullmode = Cullmode::none;
		std::cout << "Cullmode changed to none" << std::endl;
		break;
	case Cullmode::none:
		m_CurrentCullmode = Cullmode::backFace;
		std::cout << "Cullmode changed to back face culling" << std::endl;
		break;
	}
}

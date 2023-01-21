#include "pch.h"
#include "RenderManager.h"
#include "Utils.h"
#include "DataTypes.h"

namespace dae {

	RenderManager::RenderManager(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize Camera
		m_pCamera = new Camera();

		int width{}, height{};
		SDL_GetWindowSize(pWindow, &width, &height);
		float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

		m_pCamera->Initialize(45.f, { .0f, 0.f, 0.f }, aspectRatio);
		
		//Initialize meshes
		LoadMeshes();

		//Create the different renderers
		m_pRendererSoftware = new Renderer_Software(pWindow, m_pCamera, m_pMeshes);
		m_pRendererHardware = new Renderer_Hardware(pWindow, m_pCamera, m_pMeshes);
		m_pCurrentRenderer = m_pRendererSoftware;
	}

	RenderManager::~RenderManager()
	{
		for (auto pMesh : m_pMeshes) {
			delete pMesh;
			pMesh = nullptr;
		}

		delete m_pCamera;
		m_pCamera = nullptr;

		delete m_pRendererSoftware;
		m_pRendererSoftware = nullptr;

		delete m_pRendererHardware;
		m_pRendererHardware = nullptr;
	}

	void RenderManager::Update(const Timer* pTimer)
	{
		//Check if the current Renderer is not a nullptr
		if (m_pCurrentRenderer) {
			m_pCurrentRenderer->Update(pTimer);
		}
	}


	void RenderManager::Render() const
	{
		//Check if the current Renderer is not a nullptr
		if (m_pCurrentRenderer) {
			m_pCurrentRenderer->Render();
		}
	}
	void RenderManager::ToggleRenderType()
	{
		switch (m_CurrentRenderType)
		{
		case RenderType::Software:
			m_pCurrentRenderer = m_pRendererHardware;
			m_CurrentRenderType = RenderType::Hardware;
			std::cout << "Switched to hardware renderer" << std::endl;
			break;
		case RenderType::Hardware:
			m_pCurrentRenderer = m_pRendererSoftware;
			m_CurrentRenderType = RenderType::Software;
			std::cout << "Switched to software renderer" << std::endl;
			break;
		}
	}

	void RenderManager::ToggleRotation()
	{
		//Turn it off for both
		if (m_pRendererSoftware->CanRotate()) {
			std::cout << "Rotation turned off" << std::endl;
		}
		else {
			std::cout << "Rotation turned on" << std::endl;
		}
		m_pRendererSoftware->ToggleRotation();
		m_pRendererHardware->ToggleRotation();
	}

	void RenderManager::ToggleFire()
	{
		//Only if you are currently in hardware
		if (m_pCurrentRenderer == m_pRendererHardware) {
			if (m_pRendererHardware->CanRenderFire()) {
				std::cout << "Fire turned off" << std::endl;
			}
			else {
				std::cout << "Fire turned on" << std::endl;
			}
			m_pRendererHardware->ToggleFire();
		}
	}

	void RenderManager::CycleSamplerState()
	{
		//Only if you are currently in hardware
		if (m_pCurrentRenderer == m_pRendererHardware) {
			m_pRendererHardware->CycleSamplerState();
		}
	}

	void RenderManager::CycleShadingMode()
	{
		//Only if you are in software
		if (m_pCurrentRenderer == m_pRendererSoftware) {
			m_pRendererSoftware->CycleLightingMode();
		}
	}

	void RenderManager::ToggleNormalMap()
	{
		//Only if you are in software
		if (m_pCurrentRenderer == m_pRendererSoftware) {
			m_pRendererSoftware->ToggleNormalMap();
		}
	}

	void RenderManager::ToggleDepthBuffer()
	{
		//Only if you are in software
		if (m_pCurrentRenderer == m_pRendererSoftware) {
			m_pRendererSoftware->ToggleDepthBuffer();
		}
	}

	void RenderManager::ToggleBoundingBox()
	{
		//Only if you are in software
		if (m_pCurrentRenderer == m_pRendererSoftware) {
			m_pRendererSoftware->ToggleBoundingBox();
		}
	}

	void RenderManager::TogglePrintFPW()
	{
		m_CanPrintFPW = !m_CanPrintFPW;
	}

	void RenderManager::ToggleClearColor()
	{
		//Toggle clear color for both versions
		m_pRendererHardware->ToggleClearColor();
		m_pRendererSoftware->ToggleClearColor();
	}

	void RenderManager::CycleCullMode()
	{
		//Cycle cull mode for both versions
		m_pRendererHardware->CycleCullmode();
		m_pRendererSoftware->CycleCullmode();
	}

	bool RenderManager::CanPrintFPW()
	{
		return m_CanPrintFPW;
	}

	void RenderManager::LoadMeshes()
	{
		//Initial transform
		Vector3 translation{ 0, 0, 50.f };
		Vector3 scale{ 1, 1, 1 };
		float yawRotation = 90.f * TO_RADIANS;
		
		//Vehicle
		//vertices, indices, translation, scale, rotation, textures
		std::vector<Vertex_In> vertices{};
		std::vector<uint32_t> indices{};
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);
		//load needed textures
		std::vector<Texture*> pTextures{};
		Texture* pDiffuse = Texture::LoadFromFile("Resources/vehicle_diffuse.png");
		Texture* pNormal = Texture::LoadFromFile("Resources/vehicle_normal.png");
		Texture* pSpecular = Texture::LoadFromFile("Resources/vehicle_specular.png");
		Texture* pGloss = Texture::LoadFromFile("Resources/vehicle_gloss.png");
		pTextures.push_back(pDiffuse);
		pTextures.push_back(pNormal);
		pTextures.push_back(pSpecular);
		pTextures.push_back(pGloss);

		m_pMeshes.push_back(new Mesh(vertices, indices, translation, scale, yawRotation, pTextures));

		//Fire
		//vertices, indices, translation, scale, rotation, textures
		std::vector<Vertex_In> fireVertices{};
		std::vector<uint32_t> fireIndices{};
		Utils::ParseOBJ("Resources/fireFX.obj", fireVertices, fireIndices);

		//load needed textures
		std::vector<Texture*> pFireTextures{};
		Texture* pFireDiffuse = Texture::LoadFromFile("Resources/fireFX_diffuse.png");
		pFireTextures.push_back(pFireDiffuse);

		m_pMeshes.push_back(new Mesh(fireVertices, fireIndices, translation, scale, yawRotation, pFireTextures));
	}
}

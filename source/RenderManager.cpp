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

		m_pCamera->Initialize(90.f, { .0f, 0.f, 0.f }, aspectRatio);
		
		//Initialize meshes
		LoadMeshes();

		//Create the different renderers
		m_pRendererSoftware = new Renderer_Software(pWindow, m_pCamera, m_pMeshes);
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
	void RenderManager::LoadMeshes()
	{
		//Vehicle
		//vertices, indices, translation, scale, rotation, textures
		std::vector<Vertex_In> vertices{};
		std::vector<uint32_t> indices{};
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);
		Vector3 translation{0, 0, 50.f};
		Vector3 scale{ 1, 1, 1 };
		float yawRotation = 90.f * TO_RADIANS;
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
	}
}

#pragma once
#include "Renderer.h"
#include "DataTypes.h"
#include "Texture.h"

struct SDL_Surface;

class Renderer_Software final: public Renderer{
public:
	Renderer_Software(SDL_Window* pWindow, dae::Camera* pCamera, std::vector<Mesh*>& pMeshes);
	virtual ~Renderer_Software() override;

	//Rule of 5
	Renderer_Software(const Renderer_Software&) = delete;
	Renderer_Software(Renderer_Software&&) noexcept = delete;
	Renderer_Software& operator=(const Renderer_Software&) = delete;
	Renderer_Software& operator=(Renderer_Software&&) noexcept = delete;

	//Pure virtual functions
	virtual void Update(const dae::Timer* pTimer) override;
	virtual void Render() override;

	enum class ShadingMode
	{
		ObservedArea,
		Diffuse,
		Specular,
		Combined,
	};

	void ToggleDepthBuffer();
	void ToggleRotation();
	void ToggleNormalMap();
	bool SaveBufferToImage() const;
	void CycleLightingMode();
	void ToggleBoundingBox();
	bool CanRotate();

private:
	//Window in base class

	SDL_Surface* m_pFrontBuffer{ nullptr };
	SDL_Surface* m_pBackBuffer{ nullptr };
	uint32_t* m_pBackBufferPixels{};

	float* m_pDepthBufferPixels{};

	//Camera and base meshes in base class
	std::vector<Mesh_Software*> m_pSoftwareMeshes{};
	std::vector<Light*> m_pLights{};

	bool m_RenderDepthBuffer{};
	bool m_CanUseNormalMap{ true };
	bool m_CanRenderBoundingBox{ false };

	ShadingMode m_CurrentShadingMode{ ShadingMode::Combined };

	void Render_Meshes();
	void RenderTriangle(const Vertex_Out& vertex1, const Vertex_Out& vertex2, const Vertex_Out& vertex3, Mesh_Software* pSoftwareMesh);

	ColorRGB PixelShading(const Vertex_Out& vertex, Mesh_Software* pMesh);

	//Function that transforms the vertices from the mesh from World space to Screen space
	void MeshVertexTransformationFunction(std::vector<Mesh_Software*>& meshes_in) const;
};

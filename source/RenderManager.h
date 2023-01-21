#pragma once

#include "Renderer_Software.h"
#include "Renderer_Hardware.h"
#include "Texture.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class RenderManager final
	{
	public:
		RenderManager(SDL_Window* pWindow);
		~RenderManager();

		RenderManager(const RenderManager&) = delete;
		RenderManager(RenderManager&&) noexcept = delete;
		RenderManager& operator=(const RenderManager&) = delete;
		RenderManager& operator=(RenderManager&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

	private:
		SDL_Window* m_pWindow{};
		Renderer_Software* m_pRendererSoftware{};
		Renderer_Hardware* m_pRendererHardware{};

		Renderer* m_pCurrentRenderer{};
		Camera* m_pCamera{};

		std::vector<Mesh*> m_pMeshes{};

		void LoadMeshes();
	};
}

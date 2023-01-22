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

		void ToggleRenderType();
		void ToggleRotation();
		void ToggleFire();
		void CycleSamplerState();
		void CycleShadingMode();
		void ToggleNormalMap();
		void ToggleDepthBuffer();
		void ToggleBoundingBox();
		void TogglePrintFPW();
		void ToggleClearColor();
		void CycleCullMode();

		bool CanPrintFPW();

		enum class RenderType {
			Software,
			Hardware
		};

	private:
		bool m_CanPrintFPW{ false };

		SDL_Window* m_pWindow{};
		Renderer_Software* m_pRendererSoftware{};
		Renderer_Hardware* m_pRendererHardware{};

		Renderer* m_pCurrentRenderer{};
		Camera* m_pCamera{};

		std::vector<Mesh*> m_pMeshes{};

		RenderType m_CurrentRenderType{ RenderType::Software };

		void LoadMeshes();
		void PrintInfo();
	};
}

#pragma once

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

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		//DIRECTX
		HRESULT InitializeDirectX();
		//...
	};
}

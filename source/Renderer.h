#pragma once

#include <cstdint>
#include <vector>

struct SDL_Window;

namespace dae
{
	class Timer;
	class Camera;
	class Mesh;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		virtual ~Renderer() = default;

		//Rule of 5
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		//Pure virtual functions
		virtual void Update(Timer* pTimer) = 0;
		virtual void Render() = 0;

	private:
		SDL_Window* m_pWindow{};
		
		int m_Width{};
		int m_Height{};

		Camera* m_pCamera{};
		std::vector<Mesh*> m_pMeshes{};

		ColorRGB m_RendererColor{};
		ColorRGB m_UniformColor{};

		bool m_ShouldUseUniformColor{ false };
	};
}

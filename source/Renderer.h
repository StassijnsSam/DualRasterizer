#pragma once

#include <vector>
#include "Timer.h"
#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;

class Timer;
class Camera;

class Renderer
{
public:
	Renderer(SDL_Window* pWindow, dae::Camera* pCamera, std::vector<Mesh*> pMeshes);
	virtual ~Renderer() = default;

	//Rule of 5
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) noexcept = delete;

	enum class Cullmode {
		backFace,
		frontFace,
		none
	};

	void ToggleRotation();
	void ToggleClearColor();
	virtual void CycleCullmode();

	//Pure virtual functions
	virtual void Update(const dae::Timer* pTimer) = 0;
	virtual void Render() = 0;

protected:
	SDL_Window* m_pWindow{};
		
	int m_Width{};
	int m_Height{};

	float m_AspectRatio{};

	dae::Camera* m_pCamera{};
	std::vector<Mesh*> m_pMeshes{};

	dae::ColorRGB m_RendererColor{};
	dae::ColorRGB m_UniformColor{ 0.1f, 0.1f, 0.1f };

	bool m_ShouldUseUniformColor{ false };
	bool m_CanRotate{ true };

	Cullmode m_CurrentCullmode{ Cullmode::backFace };
};

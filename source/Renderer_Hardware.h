#pragma once
#include "pch.h"
#include "DataTypes.h"
#include "Texture.h"
#include "Camera.h"
#include "Renderer.h"
#include "Effect.h"
#include "Mesh_Hardware.h"

struct SDL_Window;
struct SDL_Surface;

class Renderer_Hardware final : public Renderer {
public:
	Renderer_Hardware(SDL_Window* pWindow, dae::Camera* pCamera, std::vector<Mesh*>& pMeshes);
	virtual ~Renderer_Hardware() override;

	//Rule of 5
	Renderer_Hardware(const Renderer_Hardware&) = delete;
	Renderer_Hardware(Renderer_Hardware&&) noexcept = delete;
	Renderer_Hardware& operator=(const Renderer_Hardware&) = delete;
	Renderer_Hardware& operator=(Renderer_Hardware&&) noexcept = delete;

	//Pure virtual functions
	virtual void Update(const dae::Timer* pTimer) override;
	virtual void Render() override;

	void CycleSamplerState();
	virtual void CycleCullmode() override;
	void ToggleFire();
	bool CanRenderFire();

private:
	//Window in base class
	bool m_IsInitialized{ false };
	bool m_CanRenderFire{ true };

	//DIRECTX
	HRESULT InitializeDirectX();

	ID3D11Device* m_pDevice{};
	ID3D11DeviceContext* m_pDeviceContext{};
	IDXGISwapChain* m_pSwapChain{};
	ID3D11Texture2D* m_pDepthStencilBuffer{};
	ID3D11DepthStencilView* m_pDepthStencilView{};
	ID3D11Resource* m_pRenderTargetBuffer{};
	ID3D11RenderTargetView* m_pRenderTargetView{};

	//Camera and base meshes in base class

	std::vector<Mesh_Hardware*> m_pHardwareMeshes{};
	EffectPosTex* m_pEffectPosTex{};
	EffectPosTransp* m_pEffectFire{};
};
#pragma once

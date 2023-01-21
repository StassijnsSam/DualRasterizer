#include "pch.h"
#include "Renderer_Hardware.h"
#include "Utils.h"

Renderer_Hardware::Renderer_Hardware(SDL_Window* pWindow, dae::Camera* pCamera, std::vector<Mesh*>& pMeshes) :
	Renderer(pWindow, pCamera, pMeshes)
{
	m_RendererColor = ColorRGB{ 0.39f, 0.59f, 0.93f };
	m_UniformColor = ColorRGB{ 0.1f, 0.1f, 0.1f };
	
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Initialize DirectX pipeline
	const HRESULT result = InitializeDirectX();
	if (result == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is initialized and ready!\n";
	}
	else
	{
		std::cout << "DirectX initialization failed!\n";
	}
	//pMeshes should contain 2 meshes
	// 1 is vehicle, 2 is the fire
	//Initialize all the DirectX resources for the textures
	for (auto mesh : m_pMeshes) {
		for (auto texture : mesh->pTextures) {
			texture->CreateDirectXResources(m_pDevice);
		}
	}

	//Initialize Vehicle Effect
	m_pEffectPosTex = new EffectPosTex(m_pDevice);
	m_pEffectPosTex->Initialize();
	m_pEffectPosTex->SetDiffuseMap(pMeshes[0]->pTextures[0]);
	m_pEffectPosTex->SetNormalMap(pMeshes[0]->pTextures[1]);
	m_pEffectPosTex->SetSpecularMap(pMeshes[0]->pTextures[2]);
	m_pEffectPosTex->SetGlossinessMap(pMeshes[0]->pTextures[3]);
	//Initialize Fire Effect
	m_pEffectFire = new EffectPosTransp(m_pDevice);
	m_pEffectFire->Initialize();
	m_pEffectFire->SetDiffuseMap(pMeshes[1]->pTextures[0]);
	
	//Initialize meshes
	Mesh_Hardware* pVehicle = new Mesh_Hardware(pMeshes[0], m_pDevice, m_pEffectPosTex);
	Mesh_Hardware* pFire = new Mesh_Hardware(pMeshes[1], m_pDevice, m_pEffectFire);
	m_pHardwareMeshes.push_back(pVehicle);
	m_pHardwareMeshes.push_back(pFire);
}

Renderer_Hardware::~Renderer_Hardware()
{
	// Release in the reverse order

	if (m_pRenderTargetView) {
		m_pRenderTargetView->Release();
	}
	if (m_pRenderTargetBuffer) {
		m_pRenderTargetBuffer->Release();
	}
	if (m_pDepthStencilView) {
		m_pDepthStencilView->Release();
	}
	if (m_pDepthStencilBuffer) {
		m_pDepthStencilBuffer->Release();
	}
	if (m_pSwapChain) {
		m_pSwapChain->Release();
	}
	if (m_pDeviceContext) {
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}
	if (m_pDevice) {
		m_pDevice->Release();
	}
	if (m_pEffectPosTex) {
		delete m_pEffectPosTex;
		m_pEffectPosTex = nullptr;
	}
	if (m_pEffectFire) {
		delete m_pEffectFire;
		m_pEffectFire = nullptr;
	}
	for (auto pMesh : m_pHardwareMeshes) {
		delete pMesh;
		pMesh = nullptr;
	}
}

void Renderer_Hardware::Update(const dae::Timer* pTimer)
{
	m_pCamera->Update(pTimer);
	if (m_CanRotate) {
		for (const auto pMesh : m_pMeshes) {
			pMesh->RotateY(PI_DIV_4 * pTimer->GetElapsed());
		}
	}
}


void Renderer_Hardware::Render()
{
	if (!m_IsInitialized)
		return;

	// Clear RTV and DSV

	ColorRGB clearColor = m_RendererColor;
	if (m_ShouldUseUniformColor) {
		clearColor = m_UniformColor;
	}
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	// Set Pipeline and Invoke DrawCalls (= RENDER)
	for (int i{}; i < m_pHardwareMeshes.size(); ++i) {
		Mesh_Hardware* pHardwareMesh = m_pHardwareMeshes[i];
		Mesh* pMesh = pHardwareMesh->GetInternalMesh();
		dae::Matrix worldMatrix = pMesh->worldMatrix;
		dae::Matrix worldViewProjectionMatrix = worldMatrix * m_pCamera->viewMatrix * m_pCamera->projectionMatrix;
		pHardwareMesh->SetInvViewMatrix(m_pCamera->invViewMatrix);
		pHardwareMesh->SetWorldMatrix(worldMatrix);
		pHardwareMesh->SetWorldViewProjectionMatrix(worldViewProjectionMatrix);
		//1 will always be the fire mesh
		if (i == 1) {
			if (!m_CanRenderFire) {
				continue;
			}
		}
		pHardwareMesh->Render(m_pDeviceContext);
	}
	// Update all the matrices

	// Present backbuffer (SWAP)
	m_pSwapChain->Present(0, 0);
}

void Renderer_Hardware::CycleSamplerState() {
	if (m_pEffectPosTex) {
		m_pEffectPosTex->CycleSamplerState();
	}
}

void Renderer_Hardware::ToggleFire()
{
	m_CanRenderFire = !m_CanRenderFire;
}

bool Renderer_Hardware::CanRenderFire()
{
	return m_CanRenderFire;
}

HRESULT Renderer_Hardware::InitializeDirectX()
{
	// Create Device & DeviceContext
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	uint32_t createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
		1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

	if (FAILED(result)) {
		return result;
	}

	// Create DXGI Factory
	IDXGIFactory1* pDXGIFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDXGIFactory));

	if (FAILED(result)) {
		return result;
	}

	// Create swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	// Get the handle (HWND) from the SDL Backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	result = pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

	if (FAILED(result)) {
		return result;
	}

	// Create DepthStencil and DepthStencilView
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);

	if (FAILED(result)) {
		return result;
	}

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);

	if (FAILED(result)) {
		return result;
	}

	// Create RenderTarget and RenderTargetView

	// Resource
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));

	if (FAILED(result)) {
		return result;
	}

	// View
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);

	if (FAILED(result)) {
		return result;
	}

	// Bind RTX and DSV to Output Merger Stage

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// Set Viewport
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(m_Width);
	viewport.Height = static_cast<float>(m_Height);
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	return result;
}

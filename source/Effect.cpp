#include "pch.h"
#include "Effect.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& path):
	m_pDevice{pDevice},
	m_EffectFile{path}
{
}

Effect::~Effect()
{
	if (m_pSamplerState) {
		m_pSamplerState->Release();
	}
	if (m_pPointSamplerState) {
		m_pPointSamplerState->Release();
	}
	if (m_pLinearSamplerState) {
		m_pLinearSamplerState->Release();
	}
	if (m_pBackfaceCullingState) {
		m_pBackfaceCullingState->Release();
	}
	if (m_pFrontfaceCullingState) {
		m_pFrontfaceCullingState->Release();
	}
	if (m_pNoCullingState) {
		m_pNoCullingState->Release();
	}
	if (m_pAnisotropicSamplerState) {
		m_pAnisotropicSamplerState->Release();
	}
	if (m_pEffectTechnique) {
		m_pEffectTechnique->Release();
	}
	if (m_pEffect) {
		m_pEffect->Release();
	}
	if (m_pInputLayout) {
		m_pInputLayout->Release();
	}
}

ID3DX11Effect* Effect::GetEffect()
{
	return m_pEffect;
}

ID3DX11EffectTechnique* Effect::GetEffectTechnique()
{
	return m_pEffectTechnique;
}

ID3DX11EffectMatrixVariable* Effect::GetMatrixVariable()
{
	return m_pMatWorldViewProjVariable;
}

ID3D11InputLayout* Effect::GetInputLayout()
{
	return m_pInputLayout;
}

void Effect::Initialize()
{
	m_pEffect = LoadEffect(m_pDevice, m_EffectFile);

	m_pEffectTechnique = m_pEffect->GetTechniqueByIndex(0);
	if (!(m_pEffectTechnique->IsValid())) {
		std::wcout << L"Technique is not valid!" << std::endl;
	}
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!(m_pMatWorldViewProjVariable->IsValid())) {
		std::wcout << L"MatWorldViewProjVariable is not valid!" << std::endl;
	}
	m_pSamplerState = m_pEffect->GetVariableByName("gSamplerState")->AsSampler();
	if (!(m_pSamplerState->IsValid()))
	{
		std::wcout << L"SamplerState variable not found!\n";
	}

	m_pRasterizerState = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();
	if (!(m_pRasterizerState->IsValid()))
	{
		std::wcout << L"RasterizerState variable not found!\n";
	}

	// Create sampler states
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 1;

	// Point Sampler
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	HRESULT hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pPointSamplerState);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create point sampler");
	}
	// Linear Sampler
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pLinearSamplerState);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create linear sampler");
	}
	// Anisotropic Sampler
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pAnisotropicSamplerState);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create anisotropic sampler");
	}

	// Initially set to point
	if (m_pPointSamplerState) {
		m_pSamplerState->SetSampler(0, m_pPointSamplerState);
		m_CurrentSampleState = SampleState::Point;
	}

	//Create rasterizer states
	D3D11_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	//Back face
	hr = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pBackfaceCullingState);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create backface culling state");
	}
	//Front face
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
	hr = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pFrontfaceCullingState);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create frontface culling state");
	}
	//No culling
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	hr = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pNoCullingState);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create no culling state");
	}

	// Initially set to back face
	if (m_pBackfaceCullingState) {
		m_pRasterizerState->SetRasterizerState(0, m_pBackfaceCullingState);
		m_CurrentRasterizerState = RasterizerState::Back;
	}
	
	BuildInputLayout();
	LoadEffectVariable();
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined (_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result)) {
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++) {
				ss << pErrors[i];
			}
			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else 
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile! \n Path: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}
	return pEffect;
}

EffectPosTex::~EffectPosTex()
{
	if (m_pDiffuseMapVariable) {
		m_pDiffuseMapVariable->Release();
	}
}

void EffectPosTex::BuildInputLayout()
{
	// Create vertex layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create input layout
	D3DX11_PASS_DESC passDesc{};
	m_pEffectTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = m_pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout
	);

	if (FAILED(result)) {
		std::cout << "Failed input layout" << std::endl;
		return;
	}
}

void EffectPosTex::LoadEffectVariable()
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!(m_pDiffuseMapVariable->IsValid())) {
		std::wcout << L"DiffuseMapVariable is not valid!" << std::endl;
	}
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!(m_pNormalMapVariable->IsValid())) {
		std::wcout << L"NormalMapVariable is not valid!" << std::endl;
	}
	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!(m_pSpecularMapVariable->IsValid())) {
		std::wcout << L"SpecularMapVariable is not valid!" << std::endl;
	}
	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!(m_pGlossinessMapVariable->IsValid())) {
		std::wcout << L"GlossinessMap is not valid!" << std::endl;
	}
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!(m_pMatWorldVariable->IsValid())) {
		std::wcout << L"WorldMatrix is not valid!" << std::endl;
	}
	m_pMatViewInvVariable = m_pEffect->GetVariableByName("gViewInvMatrix")->AsMatrix();
	if (!(m_pMatViewInvVariable->IsValid())) {
		std::wcout << L"ViewInvMatrix is not valid!" << std::endl;
	}
}

void EffectPosTex::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable) {
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetResourceView());
	}
}

void EffectPosTex::SetNormalMap(Texture* pNormalTexture)
{
	if (m_pNormalMapVariable) {
		m_pNormalMapVariable->SetResource(pNormalTexture->GetResourceView());
	}
}

void EffectPosTex::SetSpecularMap(Texture* pSpecularTexture)
{
	if (m_pSpecularMapVariable) {
		m_pSpecularMapVariable->SetResource(pSpecularTexture->GetResourceView());
	}
}

void EffectPosTex::SetGlossinessMap(Texture* pGlossinessTexture)
{
	if (m_pGlossinessMapVariable) {
		m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetResourceView());
	}
}

void Effect::CycleSamplerState()
{
	switch (m_CurrentSampleState)
	{
	case SampleState::Point:
		m_pSamplerState->SetSampler(0, m_pLinearSamplerState);
		m_CurrentSampleState = SampleState::Linear;
		std::wcout << L"Current Sample State set to Linear!" << std::endl;
		break;
	case SampleState::Linear:
		m_pSamplerState->SetSampler(0, m_pAnisotropicSamplerState);
		m_CurrentSampleState = SampleState::Anisotropic;
		std::wcout << L"Current Sample State set to Anisotropic!" << std::endl;
		break;
	case SampleState::Anisotropic:
		m_pSamplerState->SetSampler(0, m_pPointSamplerState);
		m_CurrentSampleState = SampleState::Point;
		std::wcout << L"Current Sample State set to Point!" << std::endl;
		break;
	}
}

void Effect::CycleRasterizerState()
{
	switch (m_CurrentRasterizerState)
	{
	case RasterizerState::Back:
		m_pRasterizerState->SetRasterizerState(0, m_pFrontfaceCullingState);
		m_CurrentRasterizerState = RasterizerState::Front;
		break;
	case RasterizerState::Front:
		m_pRasterizerState->SetRasterizerState(0, m_pNoCullingState);
		m_CurrentRasterizerState = RasterizerState::None;
		break;
	case RasterizerState::None:
		m_pRasterizerState->SetRasterizerState(0, m_pBackfaceCullingState);
		m_CurrentRasterizerState = RasterizerState::Back;
		break;
	}
}

void EffectPosTransp::BuildInputLayout()
{
	// Create vertex layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create input layout
	D3DX11_PASS_DESC passDesc{};
	m_pEffectTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = m_pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout
	);

	if (FAILED(result)) {
		std::cout << "Failed input layout" << std::endl;
		return;
	}
}

void EffectPosTransp::LoadEffectVariable()
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!(m_pDiffuseMapVariable->IsValid())) {
		std::wcout << L"DiffuseMapVariable is not valid!" << std::endl;
	}

	//Set the default rasterizer state to none!
	m_pRasterizerState->SetRasterizerState(0, m_pNoCullingState);
	m_CurrentRasterizerState = RasterizerState::None;
}

void EffectPosTransp::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable) {
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetResourceView());
	}
}

ID3DX11EffectMatrixVariable* Effect::GetWorldMatrixVariable()
{
	return m_pMatWorldVariable;
}

ID3DX11EffectMatrixVariable* Effect::GetInViewMatrixVariable()
{
	return m_pMatViewInvVariable;
}

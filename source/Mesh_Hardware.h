#pragma once

#include "pch.h"
#include "Effect.h"
#include "Math.h"
#include "DataTypes.h"

struct Vertex {
	dae::Vector3 position{};
	dae::Vector2 color{};
};

struct Vertex_Tex {
	dae::Vector3 position{};
	dae::Vector3 normal{};
	dae::Vector3 tangent{};
	dae::Vector2 uv{};
};

struct Vertex_Transp {
	dae::Vector3 position{};
	dae::Vector2 uv{};
};

class Mesh_Hardware {
public:
	template<typename T>
	Mesh_Hardware(Mesh* pMesh, ID3D11Device* pDevice, Effect* pEffect);
	~Mesh_Hardware();

	template<typename T>
	void Render(ID3D11DeviceContext* pDeviceContext) const;

	void SetWorldMatrix(dae::Matrix worldMatrix);
	void SetInvViewMatrix(dae::Matrix invViewMatrix);
	void SetWorldViewProjectionMatrix(dae::Matrix worldViewProjectionMatrix);

	dae::Matrix GetWorldMatrix();
private:

	Mesh* m_pInternalMesh{};

	Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pEffectTechnique{};
	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};
	ID3D11InputLayout* m_pInputLayout{};

	uint32_t m_NumIndices{};
};

template<typename T>
Mesh_Hardware::Mesh_Hardware(Mesh* pMesh, ID3D11Device* pDevice, Effect* pEffect)
{
	m_pEffect = pEffect;
	m_pEffectTechnique = m_pEffect->GetEffectTechnique();
	m_pInputLayout = m_pEffect->GetInputLayout();


	// Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	// Use the template here, the sizes are different
	bd.ByteWidth = sizeof(T) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

	if (FAILED(result)) {
		return;
	}

	// Create index buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

	if (FAILED(result)) {
		return;
	}
}

template <typename T>
void Mesh_Hardware::Render(ID3D11DeviceContext* pDeviceContext) const
{
	// Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set input layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// Set vertex buffer
	constexpr UINT stride = sizeof(T);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// Update the matrices
	// Now done in Renderer

	// Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetEffectTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {
		m_pEffect->GetEffectTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

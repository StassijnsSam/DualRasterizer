#include "pch.h"
#include "Mesh_Hardware.h"

Mesh_Hardware::Mesh_Hardware(Mesh* pMesh, ID3D11Device* pDevice, Effect* pEffect)
{
	m_pEffect = pEffect;
	m_pEffectTechnique = m_pEffect->GetEffectTechnique();
	m_pInputLayout = m_pEffect->GetInputLayout();

	m_pInternalMesh = pMesh;

	// Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	// Use the template here, the sizes are different
	bd.ByteWidth = sizeof(Vertex_In) * static_cast<uint32_t>(pMesh->vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pMesh->vertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

	if (FAILED(result)) {
		return;
	}

	// Create index buffer
	m_NumIndices = static_cast<uint32_t>(pMesh->indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = pMesh->indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

	if (FAILED(result)) {
		return;
	}
}


void Mesh_Hardware::Render(ID3D11DeviceContext* pDeviceContext) const
{
	// Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set input layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// Set vertex buffer
	constexpr UINT stride = sizeof(Vertex_In);
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

Mesh_Hardware::~Mesh_Hardware()
{
	if (m_pVertexBuffer) {
		m_pVertexBuffer->Release();
	}
	if (m_pIndexBuffer) {
		m_pIndexBuffer->Release();
	}
}

Mesh* Mesh_Hardware::GetInternalMesh()
{
	return m_pInternalMesh;
}

void Mesh_Hardware::SetWorldMatrix(dae::Matrix worldMatrix)
{
	auto matVar = m_pEffect->GetWorldMatrixVariable();
	if (matVar) {
		matVar->SetMatrix(reinterpret_cast<float*>(&worldMatrix));
	}
}

void Mesh_Hardware::SetInvViewMatrix(dae::Matrix invViewMatrix)
{
	auto matVar = m_pEffect->GetInViewMatrixVariable();
	if (matVar) {
		matVar->SetMatrix(reinterpret_cast<float*>(&invViewMatrix));
	}
}

void Mesh_Hardware::SetWorldViewProjectionMatrix(dae::Matrix worldViewProjectionMatrix)
{
	auto matVar = m_pEffect->GetMatrixVariable();
	if (matVar) {
		matVar->SetMatrix(reinterpret_cast<float*>(&worldViewProjectionMatrix)); //need to cast to float*
	}
}
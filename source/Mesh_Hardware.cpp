#include "pch.h"
#include "Mesh_Hardware.h"

Mesh_Hardware::~Mesh_Hardware()
{
	if (m_pVertexBuffer) {
		m_pVertexBuffer->Release();
	}
	if (m_pIndexBuffer) {
		m_pIndexBuffer->Release();
	}
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
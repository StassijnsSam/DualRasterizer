#pragma once

#include "pch.h"
#include "Effect.h"
#include "Math.h"
#include "DataTypes.h"
class Mesh_Hardware {
public:
	Mesh_Hardware(Mesh* pMesh, ID3D11Device* pDevice, Effect* pEffect);
	~Mesh_Hardware();

	//Rule of 5
	Mesh_Hardware(const Mesh_Hardware&) = delete;
	Mesh_Hardware& operator=(const Mesh_Hardware&) = delete;
	Mesh_Hardware(Mesh_Hardware&&) = delete;
	Mesh_Hardware& operator=(Mesh_Hardware&&) noexcept = delete;

	void Render(ID3D11DeviceContext* pDeviceContext) const;

	Mesh* GetInternalMesh();

	void SetWorldMatrix(dae::Matrix worldMatrix);
	void SetInvViewMatrix(dae::Matrix invViewMatrix);
	void SetWorldViewProjectionMatrix(dae::Matrix worldViewProjectionMatrix);
private:

	Mesh* m_pInternalMesh{};

	Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pEffectTechnique{};
	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};
	ID3D11InputLayout* m_pInputLayout{};

	uint32_t m_NumIndices{};
};

#pragma once

#include "Texture.h"

using namespace dae;

class Effect {
public:
	Effect(ID3D11Device* pDevice, const std::wstring& path);
	~Effect();

	ID3DX11Effect* GetEffect();
	ID3DX11EffectTechnique* GetEffectTechnique();
	ID3DX11EffectMatrixVariable* GetMatrixVariable();
	ID3D11InputLayout* GetInputLayout();
	void Initialize();
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	virtual void SetDiffuseMap(Texture* pDiffuseTexture) = 0;
	virtual void SetNormalMap(Texture* pNormalTexture) {};
	virtual void SetSpecularMap(Texture* pSpecularTexture) {};
	virtual void SetGlossinessMap(Texture* pGlossinessTexture) {};

	enum SampleState {
		Point,
		Linear,
		Anisotropic
	};

	void CycleSamplerState();
	ID3DX11EffectMatrixVariable* GetWorldMatrixVariable();
	ID3DX11EffectMatrixVariable* GetInViewMatrixVariable();

protected:
	ID3DX11Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pEffectTechnique{};
	ID3D11InputLayout* m_pInputLayout{};

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
	//Extra matrices
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
	ID3DX11EffectMatrixVariable* m_pMatViewInvVariable{};

	std::wstring m_EffectFile{};
	ID3D11Device* m_pDevice{};
	ID3DX11EffectSamplerVariable* m_pSamplerState{};

	//Sampler states
	ID3D11SamplerState* m_pPointSamplerState{};
	ID3D11SamplerState* m_pLinearSamplerState{};
	ID3D11SamplerState* m_pAnisotropicSamplerState{};

	SampleState m_CurrentSampleState{};

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

	virtual void BuildInputLayout() = 0;
	virtual void LoadEffectVariable() = 0;

};

class EffectPosTransp : public Effect {
public:
	EffectPosTransp(ID3D11Device* pDevice) : Effect(pDevice, L"Resources/PosTransp3D.fx") {};
	~EffectPosTransp() = default;
	void BuildInputLayout() override;
	void LoadEffectVariable() override;

	void SetDiffuseMap(Texture* pDiffuseTexture) override;
private:
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
};

class EffectPosTex : public Effect {
public:
	EffectPosTex(ID3D11Device* pDevice) : Effect(pDevice, L"Resources/PosTex3D.fx") {};
	~EffectPosTex();
	void BuildInputLayout() override;
	void LoadEffectVariable() override;

	void SetDiffuseMap(Texture* pDiffuseTexture) override;
	void SetNormalMap(Texture* pNormalTexture) override;
	void SetSpecularMap(Texture* pSpecularTexture) override;
	void SetGlossinessMap(Texture* pGlossinessTexture) override;
};

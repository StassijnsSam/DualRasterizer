// Global variables
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInvMatrix : VIEWINVERSE;

Texture2D gDiffuseMap : DiffuseMap;
SamplerState gSamplerState: GlobalSamplerState;
RasterizerState gRasterizerState : GlobalRasterizeState;

BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;
};

// Input/Output Structs
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 Uv : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 Uv : TEXCOORD;
};

// Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
	output.Uv = input.Uv;
    
	return output;
}

// Pixel Shader
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float4 diffuseMapSample = gDiffuseMap.Sample(gSamplerState, input.Uv);
    return diffuseMapSample;
}

// Technique
technique11 DefaultTechnique
{
	pass P0
	{
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
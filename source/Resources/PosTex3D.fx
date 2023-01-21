// Global variables
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInvMatrix : VIEWINVERSE;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection = { 0.577f, -0.577f, 0.577f };

SamplerState gSamplerState: GlobalSamplerState;

// Constants
float LIGHTINTENSITY = 7.0f;
float SHININESS = 25.0f;
float PI = 3.1415926535f;
float3 AMBIENTCOLOR = { 0.025f, 0.025f, 0.025f };

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

RasterizerState gRasterizerState
{
    CullMode = back;
    FrontCounterClockwise = false;
};

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
    DepthWriteMask = true;
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
    float4 WorldPosition : WORLDPOSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
	float2 Uv : TEXCOORD;
};

// Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
    output.Normal = mul(input.Normal, (float3x3)gWorldMatrix);
    output.Tangent = mul(input.Tangent, (float3x3) gWorldMatrix);
    output.WorldPosition = mul(float4(input.Position, 1.0f), gWorldMatrix);
    
	output.Uv = input.Uv;
	return output;
}

// Pixel shader
float3 CalculateLambert(float kd, float3 color)
{
    return mul(color, kd / PI);
}

float CalculatePhong(float3 specular, float phongExponent, float3 lightDirection, float3 viewDirection, float3 normal)
{
    float3 r = reflect(normal, lightDirection);
    float angle = max(0, dot(r, viewDirection));
    
    return (specular * pow(angle, phongExponent));
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 finalColor = { 0, 0, 0 };
    
    float3 binormal = normalize(cross(input.Normal, input.Tangent));
    float3x3 tangentSpaceAxis = float3x3(normalize(input.Tangent), binormal, normalize(input.Normal));
    
    float3 normalMapSample = gNormalMap.Sample(gSamplerState, input.Uv).xyz;
    normalMapSample = 2.f * normalMapSample - 1.f;
    float3 tangentSpaceNormal = normalize(mul(normalMapSample, tangentSpaceAxis));

    float observedArea = dot(tangentSpaceNormal, -gLightDirection);
    
    if (observedArea < 0)
    {
        return float4(finalColor, 1);
    }

    float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInvMatrix[3].xyz);

    float3 specularMapSample = gSpecularMap.Sample(gSamplerState, input.Uv).xyz;
    float3 glossinessMapSample = gGlossinessMap.Sample(gSamplerState, input.Uv).xyz;
    float3 diffuseMapSample = gDiffuseMap.Sample(gSamplerState, input.Uv).xyz;

    float phongValue = CalculatePhong(specularMapSample, glossinessMapSample.x * SHININESS, gLightDirection, viewDirection, tangentSpaceNormal);
    float3 diffuse = CalculateLambert(1.f, diffuseMapSample);

    finalColor = (AMBIENTCOLOR + LIGHTINTENSITY * (diffuse + phongValue)) * observedArea;

    return float4(finalColor, 1);
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
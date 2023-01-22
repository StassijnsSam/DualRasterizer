// Global variables
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInvMatrix : VIEWINVERSE;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection = { 0.577f, -0.577f, 0.557f };

SamplerState gSamplerState: GlobalSamplerState;
RasterizerState gRasterizerState : GlobalRasterizeState;

// Constants
float LIGHTINTENSITY = 7.0f;
float SHININESS = 25.0f;
float PI = 3.1415926535f;
float3 AMBIENTCOLOR = { 0.025f, 0.025f, 0.025f };

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
float3 CalculateLambert(float3 color, float kd)
{
    return mul(color, kd / PI);
}

float CalculatePhong(float3 specularColor, float phongExponent, float3 normal, VS_OUTPUT input)
{
    float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInvMatrix[3].xyz);
    float3 r = -1 * gLightDirection - (2 * dot(normal, -1 * gLightDirection) * normal);
    float cosA = max(0, dot(r, viewDirection));
    
    return (specularColor * pow(cosA, phongExponent));
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 finalColor = { 0, 0, 0 };
    
    //Normal map calculations
    // Get normal from normalMap
    float3 sampledNormal = gNormalMap.Sample(gSamplerState, input.Uv).xyz;
    sampledNormal = 2.f * sampledNormal - 1.f;
    
    float3 binormal = normalize(cross(input.Normal, input.Tangent));
    float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);
  
    sampledNormal = normalize(mul(sampledNormal, tangentSpaceAxis));

    float observedArea = dot(sampledNormal, -gLightDirection);
    
    if (observedArea <= 0)
    {
        return float4(finalColor, 1);
    }

    //Calculate lambert diffuse
    float3 diffuseColor = gDiffuseMap.Sample(gSamplerState, input.Uv).xyz;
    float3 lambertDiffuse = CalculateLambert(diffuseColor, 1.f);

    //Calculate phong specular
    float3 specularColor = gSpecularMap.Sample(gSamplerState, input.Uv).xyz;
    float3 glossColor = gGlossinessMap.Sample(gSamplerState, input.Uv).xyz;
    float glossExponent = glossColor.x;
    glossExponent *= SHININESS;
    
    float phongSpecular = CalculatePhong(specularColor, glossExponent, sampledNormal, input);
    
    //finalColor
    float3 lightColor = { 1, 1, 1 };
    float3 radiance = LIGHTINTENSITY * lightColor;
    finalColor = (AMBIENTCOLOR + radiance * (lambertDiffuse) + phongSpecular) * observedArea;

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
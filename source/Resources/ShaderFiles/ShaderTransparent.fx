
// -----------------------------------------------------------------
//  Global Variables
// -----------------------------------------------------------------
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInverse : VIEWINVERSE;

Texture2D gDiffuseMap : DiffuseMap;

SamplerState gSampler; // Used to sample textures


RasterizerState gRasterizerState
{
    CullMode = none;
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
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;

    // Others are redundant because STENCILENABLE is false
    // Added for DEMO
    StencilReadMask = 0xFF;
    StencilWriteMask = 0xFF;
    
    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;
    
    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;

    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;

    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;
};


// -----------------------------------------------------------------
//  Input/Ouput structs
// -----------------------------------------------------------------

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : COLOR; // Set to COLOR in slides because you cant reuse SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

// -----------------------------------------------------------------
//  Vertex shader
// -----------------------------------------------------------------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj); // Multiply input position with the worldviewmatrix
    output.TexCoord = input.TexCoord;
    
    //output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    //output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);    
    //output.WorldPosition = mul(float4(input.Position, 1), gWorldMatrix);
    
    return output;
}

// -----------------------------------------------------------------
//  Pixel shader
// -----------------------------------------------------------------


float4 PS(VS_OUTPUT input) : SV_TARGET
{
    // Get the texture samples using the sampler
    float4 diffuseColor = gDiffuseMap.Sample(gSampler, input.TexCoord);
    return diffuseColor;
}


// -----------------------------------------------------------------
//  Technique
// -----------------------------------------------------------------
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
};

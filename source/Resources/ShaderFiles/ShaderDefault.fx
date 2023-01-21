
// -----------------------------------------------------------------
//  Global Variables
// -----------------------------------------------------------------
float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection : LightDirection = float3(0.577f, -0.577f, 0.577f);
float3 gLightColor : LightColor = float3(1.0f, 1.0f, 1.0f);
float gLightIntensity : LightIntensity = 7.0f;
float gShininess : Shininess = 25.0f;
float gAmbientColor : Shininess = float3(0.025f, 0.025f, 0.025f);

float4x4 gWorldMatrix : WORLD;
float4x4 gViewInverse : VIEWINVERSE;

SamplerState gSampler; // Used to sample textures


const float PI = 3.1415926535897932384626433832795f;



RasterizerState gRasterizerState
{
    CullMode = back;
    FrontCounterClockwise = false;
};

BlendState gBlendState
{
    BlendEnable[0] = false;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = 1;
    DepthFunc = less;
    StencilEnable = false;
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
    
    output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
    
    
    output.WorldPosition = mul(float4(input.Position, 1), gWorldMatrix);
    
    return output;
}

// -----------------------------------------------------------------
//  Pixel shader
// -----------------------------------------------------------------

float3 Phong(float3 specularColor, float phongExponent, float3 viewDirection, float3 normal)
{
    float3 reflection = reflect(gLightDirection, normal);
    float RdotV = max(0.0f, dot(reflection, viewDirection));
    return specularColor * pow(RdotV, phongExponent);
}

float3 LambertDiffuse(float kd, float3 diffuseColor)
{
    return mul(diffuseColor, kd / PI);
}


float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);
    float3 lightRadiance = gLightColor * gLightIntensity;
    
    // Get the texture samples using the sampler
    float3 diffuseColor = gDiffuseMap.Sample(gSampler, input.TexCoord).rgb;
    float3 normalSample = gNormalMap.Sample(gSampler, input.TexCoord).rgb;
    float3 specularColor = gSpecularMap.Sample(gSampler, input.TexCoord).rgb;
    float glossinessSample = gGlossinessMap.Sample(gSampler, input.TexCoord).r; // Only needs red since its a gray scale map
    
    
    // Calculate tangent space axis
    const float3 binormal = normalize(cross(input.Normal, input.Tangent));
    const float3x3 tangentSpaceAxis = float3x3(normalize(input.Tangent), binormal, normalize(input.Normal));
    
    // Calculate normal in tangent space
    //const float3 tangentNormal = float3(normalSample.r * 2.0f - 1.0f, normalSample.g * 2.0f - 1.0f, normalSample.b * 2.0f - 1.0f);
    const float3 tangentNormal = 2.f * normalSample - 1.0f;
    const float3 tangentSpaceNormal = normalize(mul(tangentNormal, tangentSpaceAxis)); // Cast to float3x3 to only get rotation
    
    // Calculate observed area / lambert consine
    const float observedArea = saturate(dot(tangentSpaceNormal, -gLightDirection));
    
    // Calculate lambert
    const float3 lambertDiffuse = LambertDiffuse(1.0f, diffuseColor);
    
    // Calculate phong
    const float3 phongSpecular = Phong(specularColor, glossinessSample * gShininess, -viewDirection, tangentSpaceNormal);
    
    // Combine all and return
    
    float3 finalColor = ((gLightColor * gLightIntensity) * lambertDiffuse + phongSpecular + gAmbientColor) * observedArea;
    //float3 finalColor =  gLightIntensity * (lambertDiffuse + phongSpecular) * observedArea;
    
    return float4(finalColor, 1.0f);
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

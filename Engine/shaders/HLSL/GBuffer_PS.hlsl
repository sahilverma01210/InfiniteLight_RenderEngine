#include "CommonResources.hlsli"

struct ImportMatCB
{
    int texConstIdx;
    int diffTexIdx;
    int normTexIdx;
    int specTexIdx;
    int solidConstIdx;
};

struct SurfaceProps
{
    float3 materialColor;
    float3 specularColor;
    float specularGloss;
    float normalMapWeight;
    bool useDiffuseAlpha;
    bool useGlossAlpha;
    bool useDiffuseMap;
    bool useNormalMap;
    bool useSpecularMap;
};

struct VSIn
{
    float2 texUV : Texcoord;
    float4 posWS : PositionWorld;
    float3 posVS : PositionView;
    float4 posCS : SV_Position;
    float3 normalVS : Normal;
    float3 tanVS : Tangent;
    float3 bitanVS : Bitangent;
};

struct PSOut
{
    float4 position : SV_Target0;
    float4 diffuse : SV_Target1;
    float4 normal : SV_Target2;
    float4 specular : SV_Target3;
};

SamplerComparisonState samplerCompareState : register(s0);
SamplerState samplerState : register(s1);

// Can use StructuredBuffer instead.

PSOut CalculatePixels(VSIn vsIn)
{
    PSOut pso;
    
    ConstantBuffer<ImportMatCB> importCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    ConstantBuffer<SurfaceProps> surfaceProps = ResourceDescriptorHeap[importCB.texConstIdx];
    
    // sample diffuse texture if defined
    float4 diffColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (surfaceProps.useDiffuseMap)
    {
        Texture2D<float4> diffTex = ResourceDescriptorHeap[importCB.diffTexIdx];
        diffColor = diffTex.Sample(samplerState, vsIn.texUV);
    }
    
    if (surfaceProps.useDiffuseAlpha)
    {
        // bail if highly translucent
        clip(diffColor.a < 0.1f ? -1 : 1);
        // flip normal when backface
        if (dot(vsIn.normalVS, vsIn.posVS) >= 0.0f)
        {
            vsIn.normalVS = -vsIn.normalVS;
        }
    }
    
    // normalize the mesh normal
    vsIn.normalVS = normalize(vsIn.normalVS);
    
    // replace normal with mapped if normal mapping enabled
    if (surfaceProps.useNormalMap)
    {
        Texture2D<float4> normTex = ResourceDescriptorHeap[importCB.normTexIdx];
        const float4 normSample = normTex.Sample(samplerState, vsIn.texUV);
    
        // build the tranform (rotation) into same space as tan/bitan/normal (target space)
        const float3x3 tanToTarget = float3x3(normalize(vsIn.tanVS), normalize(vsIn.bitanVS), vsIn.normalVS);
        // sample and unpack the normal from texture into target space   
        const float3 normalSample = normTex.Sample(samplerState, vsIn.texUV).xyz;
        const float3 tanNormal = normalSample * 2.0f - 1.0f;
        // bring normal from tanspace into target space
        const float3 mappedNormal = normalize(mul(tanNormal, tanToTarget));
    
        vsIn.normalVS = lerp(vsIn.normalVS, mappedNormal, surfaceProps.normalMapWeight);
    }
    
    // specular parameter determination (mapped or uniform)
    float4 specColor = float4(surfaceProps.specularColor, 0.0f);
    specColor.a = surfaceProps.specularGloss;
    
    if (surfaceProps.useSpecularMap)
    {
        Texture2D<float4> specTex = ResourceDescriptorHeap[importCB.specTexIdx];
        const float4 specularSample = specTex.Sample(samplerState, vsIn.texUV);
        //specColor.rgb = specularSample.rgb;
             
        if (surfaceProps.useGlossAlpha)
        {
            specColor.a = pow(2.0f, specularSample.a * 13.0f);
        }
    }
    
    if (!surfaceProps.useDiffuseMap)
    {
        diffColor.rgb = surfaceProps.materialColor;
    }   
    
    pso.position = float4(vsIn.posWS.xyz, 1.0f);
    pso.diffuse = float4(diffColor.rgb, 1.0f);
    pso.normal = float4(vsIn.normalVS, 1.0f);
    pso.specular = specColor;
    
    return pso;
}

PSOut main(VSIn vsIn) : SV_Target
{
    return CalculatePixels(vsIn);
}
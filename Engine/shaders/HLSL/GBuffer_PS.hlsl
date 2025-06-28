#include "Common.hlsli"
#include "Scene.hlsli"

struct MaterialDataHandles
{
    int diffuseIdx;
    int normalIdx;
    int roughnessMetallicIdx;
    int materialConstIdx;
};

struct MaterialConstants
{
    float3 pbrBaseColorFactor;
    float pbrMetallicFactor;
    float pbrRoughnessFactor;
    float gltfAlphaCutoff;
};

struct VSOut
{
    float2 texUV : Texcoord;
    float3 normalWS : Normal;
    float3 tanWS : Tangent;
    float3 bitanWS : Bitangent;
    float4 posCS : SV_Position;
};

struct PSOut
{
    float4 diffuse : SV_Target0;
    float4 normal : SV_Target1;
    float4 metallicRough : SV_Target2;
};

// Can use StructuredBuffer instead.

PSOut CalculatePixels(VSOut vsIn)
{
    PSOut pso;
    
    ConstantBuffer<MaterialDataHandles> matDataHandles = ResourceDescriptorHeap[meshConstants.materialIdx];
    ConstantBuffer<MaterialConstants> materialConstants = ResourceDescriptorHeap[matDataHandles.materialConstIdx];
    
    Texture2D<float4> diffTex = ResourceDescriptorHeap[matDataHandles.diffuseIdx];
    Texture2D<float4> normTex = ResourceDescriptorHeap[matDataHandles.normalIdx];
    Texture2D<float4> roughMetallicTex = ResourceDescriptorHeap[matDataHandles.roughnessMetallicIdx];
        
    float4 albedoColor = diffTex.Sample(LinearWrapSampler, vsIn.texUV) * float4(materialConstants.pbrBaseColorFactor, 1.0f);;
    if (albedoColor.a < materialConstants.gltfAlphaCutoff)
        discard;
    
    float4x4 meshView = mul(GetMeshMat(), GetViewMat());
    
    float3 normal = normalize(vsIn.normalWS);
    float3 tangent = normalize(vsIn.tanWS);
    float3 bitangent = normalize(vsIn.bitanWS);
    float3 normalTS = normalize(normTex.Sample(LinearWrapSampler, vsIn.texUV).xyz * 2.0f - 1.0f);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    normal = normalize(mul(normalTS, TBN));

    float3 aoRoughnessMetallic = roughMetallicTex.Sample(LinearWrapSampler, vsIn.texUV).rgb;
    float3 viewNormal = normalize(mul(normal, (float3x3) meshView));
    float roughness = aoRoughnessMetallic.g * materialConstants.pbrRoughnessFactor;
    float metallic = aoRoughnessMetallic.b * materialConstants.pbrMetallicFactor;
    uint shadingExtension = 0;
	
    pso.normal = EncodeGBufferNormalRT(viewNormal, metallic, shadingExtension);
    pso.diffuse = float4(albedoColor.xyz * materialConstants.pbrBaseColorFactor, roughness);
    pso.metallicRough = roughMetallicTex.Sample(LinearWrapSampler, vsIn.texUV);
    
    return pso;
}

PSOut main(VSOut vsIn) : SV_Target
{
    return CalculatePixels(vsIn);
}
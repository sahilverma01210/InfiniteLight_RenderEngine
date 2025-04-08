#include "CommonResources.hlsli"
#include "PixelShaderUtils.hlsli"
#include "LightShadowUtils.hlsli"

struct ImportMatCB
{
    int lightConstIdx;
    int texConstIdx;
    int shadowTexIdx;
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
    float specularWeight;
    float normalMapWeight;
    bool useDiffuseAlpha;
    bool useGlossAlpha;
    bool useDiffuseMap;
    bool useNormalMap;
    bool useSpecularMap;
};

struct VSIn
{
    float3 viewFragPos : Position;
    float3 viewNormal : Normal;
    float3 viewTan : Tangent;
    float3 viewBitan : Bitangent;
    float2 texUV : Texcoord;
    float4 shadowPos : ShadowPosition;
};

struct PSOut
{
    float4 pixels : SV_Target0;
    float4 shadow : SV_Target1;
    float4 diffuse : SV_Target2;
    float4 normal : SV_Target3;
    float4 specular : SV_Target4;
};

SamplerComparisonState samplerCompareState : register(s0);
SamplerState samplerState : register(s1);

// Can use StructuredBuffer instead.

PSOut CalculatePixels(VSIn vsIn)
{
    PSOut pso;
    
    ConstantBuffer<ImportMatCB> importCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    ConstantBuffer<PointLightProps> pointLightCB = ResourceDescriptorHeap[importCB.lightConstIdx];
    ConstantBuffer<SurfaceProps> surfaceProps = ResourceDescriptorHeap[importCB.texConstIdx];
    
    TextureCube smap = ResourceDescriptorHeap[importCB.shadowTexIdx];
    
    float3 diffuse;
    float3 specularReflected;
    float3 specular;
    
    // sample diffuse texture if defined
    float4 dtex = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (surfaceProps.useDiffuseMap)
    {        
        Texture2D<float4> diffTex = ResourceDescriptorHeap[importCB.diffTexIdx];
        dtex = diffTex.Sample(samplerState, vsIn.texUV);
        pso.diffuse = dtex;
    }
    
    if (surfaceProps.useDiffuseAlpha)
    {
        // bail if highly translucent
        clip(dtex.a < 0.1f ? -1 : 1);
        // flip normal when backface
        if (dot(vsIn.viewNormal, vsIn.viewFragPos) >= 0.0f)
        {
            vsIn.viewNormal = -vsIn.viewNormal;
        }
    }
    
    // shadow map test
    const float shadowLevel = smap.SampleCmpLevelZero(samplerCompareState, vsIn.shadowPos.xyz, CalculateShadowDepth(vsIn.shadowPos));
    if (shadowLevel != 0.0f)
    {
        // normalize the mesh normal
        vsIn.viewNormal = normalize(vsIn.viewNormal);
        
        // replace normal with mapped if normal mapping enabled
        if (surfaceProps.useNormalMap)
        {
            Texture2D<float4> normTex = ResourceDescriptorHeap[importCB.normTexIdx];
            const float4 normSample = normTex.Sample(samplerState, vsIn.texUV);
            pso.normal = normSample;
            const float3 mappedNormal = MapNormal(normalize(vsIn.viewTan), normalize(vsIn.viewBitan), vsIn.viewNormal, vsIn.texUV, normTex, samplerState);
            vsIn.viewNormal = lerp(vsIn.viewNormal, mappedNormal, surfaceProps.normalMapWeight);
        }

        // fragment to light vector data
        const LightVector lv = CalculateLightVector(pointLightCB.viewPos, vsIn.viewFragPos);
        
        // specular parameter determination (mapped or uniform)
        float3 specularReflectionColor = surfaceProps.specularColor;
        float specularPower = surfaceProps.specularGloss;
        
        if(surfaceProps.useSpecularMap)
        {
            Texture2D<float4> specTex = ResourceDescriptorHeap[importCB.specTexIdx];
            const float4 specularSample = specTex.Sample(samplerState, vsIn.texUV);
            pso.specular = specularSample;
            //specularReflectionColor = specularSample.rgb;
                    
            if (surfaceProps.useGlossAlpha)
            {
                specularPower = pow(2.0f, specularSample.a * 13.0f);
            }            
        }

        // attenuation
        const float attenuation = 1.0f / (pointLightCB.attConst + pointLightCB.attLin * lv.distToL + pointLightCB.attQuad * (lv.distToL * lv.distToL));
        
        // diffuse light
        diffuse = pointLightCB.diffuseColor * pointLightCB.diffuseIntensity * attenuation * max(0.0f, dot(lv.dirToL, vsIn.viewNormal));
        
        // specular reflected
        specularReflected = Speculate(
            pointLightCB.diffuseColor * pointLightCB.diffuseIntensity * specularReflectionColor, surfaceProps.specularWeight, vsIn.viewNormal,
            lv.vToL, vsIn.viewFragPos, attenuation, specularPower
        );
        
        // scale by shadow level
        diffuse *= shadowLevel;
        specularReflected *= shadowLevel;
    }
    else
    {
        diffuse = specularReflected = 0.0f;
    }

    // final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    if (surfaceProps.useDiffuseMap)
        pso.pixels = float4(saturate((diffuse + pointLightCB.ambient) * dtex.rgb + specularReflected), 1.0f);
    else
        pso.pixels = float4(saturate((diffuse + pointLightCB.ambient) * surfaceProps.materialColor + specularReflected), 1.0f);
    
    return pso;
}

PSOut main(VSIn vsIn) : SV_Target
{
    return CalculatePixels(vsIn);
}
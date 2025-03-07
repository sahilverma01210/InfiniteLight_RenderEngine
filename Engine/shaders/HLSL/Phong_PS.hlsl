#include "CommonResources.hlsl"
#include "PixelShaderUtils.hlsl"
#include "LightShadowUtils.hlsl"

struct PhongCB
{
    int shadowConstIdx;
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

SamplerComparisonState samplerCompareState : register(s0);
SamplerState samplerState : register(s1);

// Can use StructuredBuffer instead.

float4 CalculatePixels(VSIn vsIn)
{
    ConstantBuffer<PhongCB> phongCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    
    ConstantBuffer<PointLightProps> pointLightCB = ResourceDescriptorHeap[phongCB.lightConstIdx];
    ConstantBuffer<SurfaceProps> surfaceProps = ResourceDescriptorHeap[phongCB.texConstIdx];
    
    TextureCube smap = ResourceDescriptorHeap[phongCB.shadowTexIdx];
    
    int temp = meshConstants.materialIdx;
    int texIndex = 0;
    
    float3 diffuse;
    float3 specularReflected;
    float3 specular;
    
    // sample diffuse texture if defined
    float4 dtex = float4(1.0f, 1.0f, temp, 1.0f);
    if (surfaceProps.useDiffuseMap)
    {        
        Texture2D<float4> diffTex = ResourceDescriptorHeap[phongCB.diffTexIdx];
        dtex = diffTex.Sample(samplerState, vsIn.texUV);
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
            Texture2D<float4> normTex = ResourceDescriptorHeap[phongCB.normTexIdx];
            const float3 mappedNormal = MapNormal(normalize(vsIn.viewTan), normalize(vsIn.viewBitan), vsIn.viewNormal, vsIn.texUV, normTex, samplerState);
            vsIn.viewNormal = lerp(vsIn.viewNormal, mappedNormal, surfaceProps.normalMapWeight);
        }

        // fragment to light vector data
        const LightVector lv = CalculateLightVector(pointLightCB.viewLightPos, vsIn.viewFragPos);
        
        // specular parameter determination (mapped or uniform)
        float3 specularReflectionColor = surfaceProps.specularColor;
        float specularPower = surfaceProps.specularGloss;
        
        if(surfaceProps.useSpecularMap)
        {
            Texture2D<float4> specTex = ResourceDescriptorHeap[phongCB.specTexIdx];
            const float4 specularSample = specTex.Sample(samplerState, vsIn.texUV);
            specularReflectionColor = specularSample.rgb;
                    
            if (surfaceProps.useGlossAlpha)
            {
                specularPower = pow(2.0f, specularSample.a * 13.0f);
            }            
        }

        // attenuation
        const float att = Attenuate(pointLightCB.attConst, pointLightCB.attLin, pointLightCB.attQuad, lv.distToL);
        
        // diffuse light
        diffuse = Diffuse(pointLightCB.diffuseColor, pointLightCB.diffuseIntensity, att, lv.dirToL, vsIn.viewNormal);
        
        // specular reflected
        specularReflected = Speculate(
            pointLightCB.diffuseColor * pointLightCB.diffuseIntensity * specularReflectionColor, surfaceProps.specularWeight, vsIn.viewNormal,
            lv.vToL, vsIn.viewFragPos, att, specularPower
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
        return float4(saturate((diffuse + pointLightCB.ambient) * dtex.rgb + specularReflected), 1.0f);
    else
        return float4(saturate((diffuse + pointLightCB.ambient) * surfaceProps.materialColor + specularReflected), 1.0f);
}

float4 main(VSIn vsIn) : SV_Target
{
    return CalculatePixels(vsIn);
}
#include "PixelShaderUtils.hlsl"
#include "LightShadowUtils.hlsl"

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
    bool useSpecularMap;
    bool useNormalMap;
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

ConstantBuffer<PointLightProps> pointLightCB : register(b1);
ConstantBuffer<SurfaceProps> surfaceProps : register(b2);

TextureCube smap : register(t0);
#if defined(USE_DIFFUSE_MAP)
Texture2D tex[] : register(t1);
#endif

SamplerComparisonState ssam : register(s0);
SamplerState splr : register(s1);

float4 CalculatePixels(VSIn vsIn, SurfaceProps ObjectCBuf)
{
    int texIndex = 0;
    
    float3 diffuse;
    float3 specularReflected;
    float3 specular;
    
    // sample diffuse texture if defined
    float4 dtex = float4(1.0f, 1.0f, 1.0f, 1.0f);
#if defined(USE_DIFFUSE_MAP)
    //Texture2D<float4> diffTex = ResourceDescriptorHeap[texIndex++];
    dtex = tex[texIndex++].Sample(splr, vsIn.texUV);
#endif

    if (ObjectCBuf.useDiffuseAlpha)
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
    const float shadowLevel = smap.SampleCmpLevelZero(ssam, vsIn.shadowPos.xyz, CalculateShadowDepth(vsIn.shadowPos));
    if (shadowLevel != 0.0f)
    {
        // normalize the mesh normal
        vsIn.viewNormal = normalize(vsIn.viewNormal);
        
        // replace normal with mapped if normal mapping enabled
#if defined(USE_NORMAL_MAP)
        //Texture2D<float4> normTex = ResourceDescriptorHeap[texIndex++];
        const float3 mappedNormal = MapNormal(normalize(vsIn.viewTan), normalize(vsIn.viewBitan), vsIn.viewNormal, vsIn.texUV, tex[texIndex++], splr);
        vsIn.viewNormal = lerp(vsIn.viewNormal, mappedNormal, ObjectCBuf.normalMapWeight);
#endif

        // fragment to light vector data
        const LightVector lv = CalculateLightVector(pointLightCB.viewLightPos, vsIn.viewFragPos);
        
        // specular parameter determination (mapped or uniform)
        float3 specularReflectionColor = ObjectCBuf.specularColor;
        float specularPower = ObjectCBuf.specularGloss;
#if defined(USE_SPECULAR_MAP)
        //Texture2D<float4> specTex = ResourceDescriptorHeap[texIndex++];
        const float4 specularSample = tex[texIndex++].Sample(splr, vsIn.texUV);
        specularReflectionColor = specularSample.rgb;
                    
        if (ObjectCBuf.useGlossAlpha)
        {
            specularPower = pow(2.0f, specularSample.a * 13.0f);
        }
#endif

        // attenuation
        const float att = Attenuate(pointLightCB.attConst, pointLightCB.attLin, pointLightCB.attQuad, lv.distToL);
        
        // diffuse light
        diffuse = Diffuse(pointLightCB.diffuseColor, pointLightCB.diffuseIntensity, att, lv.dirToL, vsIn.viewNormal);
        
        // specular reflected
        specularReflected = Speculate(
            pointLightCB.diffuseColor * pointLightCB.diffuseIntensity * specularReflectionColor, ObjectCBuf.specularWeight, vsIn.viewNormal,
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
#if defined(USE_DIFFUSE_MAP)
    return float4(saturate((diffuse + pointLightCB.ambient) * dtex.rgb + specularReflected), 1.0f);
#else
    return float4(saturate((diffuse + pointLightCB.ambient) * ObjectCBuf.materialColor + specularReflected), 1.0f);
#endif
}

float4 main(VSIn vsIn) : SV_Target
{
    SurfaceProps props = surfaceProps;
    return CalculatePixels(vsIn, props);
}
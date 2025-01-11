#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"

#include "PointLight.hlsl"
#include "PShadow.hlsl"

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

ConstantBuffer<SurfaceProps> surfaceProps : register(b2);

SamplerState splr : register(s1);

struct VSIn
{
    float3 viewFragPos : Position;
    float3 viewNormal : Normal;
    float3 viewTan : Tangent;
    float3 viewBitan : Bitangent;
    float2 tc : Texcoord;
    float4 spos : ShadowPosition;
};

#if defined(USE_DIFFUSE_MAP) && defined(USE_NORMAL_MAP) && defined(USE_SPECULAR_MAP)
Texture2D tex : register(t1);
Texture2D spec : register(t2);
Texture2D nmap : register(t3);
#elif defined(USE_DIFFUSE_MAP) && defined(USE_SPECULAR_MAP)
Texture2D tex : register(t1);
Texture2D spec : register(t2);
#elif defined(USE_DIFFUSE_MAP) && defined(USE_NORMAL_MAP)
Texture2D tex : register(t1);
Texture2D nmap : register(t2);
#elif defined(USE_DIFFUSE_MAP)
Texture2D tex : register(t1);
#endif

float4 CalculatePixels(VSIn vsIn, SurfaceProps ObjectCBuf)
{
    float3 diffuse;
    float3 specularReflected;
    float3 specular;
    
    // sample diffuse texture if defined
    float4 dtex = float4(1.0f, 1.0f, 1.0f, 1.0f);
#if defined(USE_DIFFUSE_MAP)
    dtex = tex.Sample(splr, vsIn.tc);
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
    const float shadowLevel = Shadow(vsIn.spos);
    if (shadowLevel != 0.0f)
    {
        // normalize the mesh normal
        vsIn.viewNormal = normalize(vsIn.viewNormal);
        
        // replace normal with mapped if normal mapping enabled
#if defined(USE_NORMAL_MAP)
        if (ObjectCBuf.useNormalMap)
        {
            const float3 mappedNormal = MapNormal(normalize(vsIn.viewTan), normalize(vsIn.viewBitan), vsIn.viewNormal, vsIn.tc, nmap, splr);
            vsIn.viewNormal = lerp(vsIn.viewNormal, mappedNormal, ObjectCBuf.normalMapWeight);
        }
#endif

        // fragment to light vector data
        const LightVectorData lv = CalculateLightVectorData(viewLightPos, vsIn.viewFragPos);
        
        // specular parameter determination (mapped or uniform)
        float3 specularReflectionColor = ObjectCBuf.specularColor;
        float specularPower = ObjectCBuf.specularGloss;
#if defined(USE_SPECULAR_MAP)
        const float4 specularSample = spec.Sample(splr, vsIn.tc);
        if (ObjectCBuf.useSpecularMap)
        {
            specularReflectionColor = specularSample.rgb;
        }
        if (ObjectCBuf.useGlossAlpha)
        {
            specularPower = pow(2.0f, specularSample.a * 13.0f);
        }
#endif

        // attenuation
        const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
        
        // diffuse light
        diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, vsIn.viewNormal);
        
        // specular reflected
        specularReflected = Speculate(
            diffuseColor * diffuseIntensity * specularReflectionColor, ObjectCBuf.specularWeight, vsIn.viewNormal,
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
    return float4(saturate((diffuse + ambient) * dtex.rgb + specularReflected), 1.0f);
#else
    return float4(saturate((diffuse + ambient) * ObjectCBuf.materialColor + specularReflected), 1.0f);
#endif
}

float4 main(VSIn vsIn) : SV_Target
{
    SurfaceProps props = surfaceProps;
    return CalculatePixels(vsIn, props);
}

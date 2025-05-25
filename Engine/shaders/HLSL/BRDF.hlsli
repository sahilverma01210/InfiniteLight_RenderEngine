#ifndef  _BRDF_
#define  _BRDF_

#include "Constants.hlsli"

//add reference 

#define MIN_ROUGHNESS (0.03)

float3 DiffuseBRDF(float3 diffuse)
{
    return diffuse * M_PI_INV;
}

float D_GGX(float3 N, float3 H, float a)
{
    float a2 = clamp(a * a, 0.0001f, 1.0f);
    float NdotH = saturate(dot(N, H));
    
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;
    
    return a2 * rcp(max(denom, 0.001f));
}
float D_GGX_Aniso(float ax, float ay, float3 T, float3 B, float3 N, float3 H)
{
    float TdotH = dot(T, H);
    float BdotH = dot(B, H);
    float NdotH = dot(N, H);
    
    float a2 = ax * ay;
    float3 d = float3(ay * TdotH, ax * BdotH, a2 * NdotH);
    float d2 = dot(d, d);
    float b2 = a2 / d2;
    return a2 * b2 * b2 * M_PI_INV;
}
//http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
float V_SmithGGX(float3 N, float3 V, float3 L, float a)
{
    float a2 = clamp(a * a, 0.0001f, 1.0f);
    float NdotV = saturate(abs(dot(N, V)) + 1e-5);
    float NdotL = saturate(dot(N, L));

    float G_V = NdotV + sqrt((NdotV - NdotV * a2) * NdotV + a2);
    float G_L = NdotL + sqrt((NdotL - NdotL * a2) * NdotL + a2);
    return rcp(G_V * G_L);
}

float V_SmithJointApprox(float3 N, float3 V, float3 L, float a)
{
    float a2 = clamp(a * a, 0.0001f, 1.0f);
    float NdotV = saturate(abs(dot(N, V)) + 1e-5);
    float NdotL = saturate(dot(N, L));

    float V_SmithV = NdotL * (NdotV * (1 - a2) + a2);
    float V_SmithL = NdotV * (NdotL * (1 - a2) + a2);
    return 0.5 * rcp(V_SmithV + V_SmithL);
}


float V_SmithGGX_Aniso(float ax, float ay, float3 T, float3 B, float3 N, float3 V, float3 L)
{
    float TdotV = dot(T, V);
    float TdotL = dot(T, L);
    float BdotV = dot(B, V);
    float BdotL = dot(B, L);
    float NdotV = dot(N, V);
    float NdotL = dot(N, L);
    
    float G_V = NdotL * length(float3(ax * TdotV, ay * BdotV, NdotV));
    float G_L = NdotV * length(float3(ax * TdotL, ay * BdotL, NdotL));
    return 0.5 * rcp(G_V + G_L);
}
float3 F_Schlick(float3 V, float3 H, float3 F0)
{
    float VdotH = saturate(dot(V, H) + 1e-5);
    return F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
}
float3 SpecularBRDF(float3 N, float3 V, float3 L, float3 specular, float roughness, out float3 F)
{
    roughness = max(roughness, MIN_ROUGHNESS);

    float a = roughness * roughness;
    float3 H = normalize(V + L);

    float D = D_GGX(N, H, a);
    float Vis = V_SmithGGX(N, V, L, a);
    F = F_Schlick(V, H, specular);

    return D * Vis * F;
}
float3 DefaultBRDF(float3 L, float3 V, float3 N, float3 diffuse, float3 specular, float roughness)
{
    float3 F;
    float3 specularBrdf = SpecularBRDF(N, V, L, specular, roughness, F);
    float3 diffuseBrdf = DiffuseBRDF(diffuse) * (1.0 - F);

    return diffuseBrdf + specularBrdf;
}
float3 AnisotropyBRDF(float3 L, float3 V, float3 N, float3 T, float3 diffuse, float3 specular, float roughness, float anisotropy)
{
    float a = roughness * roughness;
    float ax = max(a * (1.0 + anisotropy), MIN_ROUGHNESS);
    float ay = max(a * (1.0 - anisotropy), MIN_ROUGHNESS);
    
    float3 B = cross(N, T);
    float3 H = normalize(V + L);
    
    float3 D = D_GGX_Aniso(ax, ay, T, B, N, H);
    float3 Vis = V_SmithGGX_Aniso(ax, ay, T, B, N, V, L);
    float3 F = F_Schlick(V, H, specular);
    
    return DiffuseBRDF(diffuse) * (1.0 - F) + D * Vis * F;
}

float DielectricSpecularToF0(float specular)
{
    return 0.08f * specular;
}
float3 ComputeF0(float specular, float3 baseColor, float metalness)
{
    return lerp(DielectricSpecularToF0(specular).xxx, baseColor, metalness);
}
float3 ComputeDiffuseColor(float3 baseColor, float metalness)
{
    return baseColor * (1 - metalness);
}

struct BrdfData
{
    float3 Diffuse;
    float3 Specular;
    float Roughness;
};


BrdfData GetBrdfData(float3 materialAlbedo, float metallic, float roughness)
{
    BrdfData data = (BrdfData) 0;
    data.Diffuse = ComputeDiffuseColor(materialAlbedo, metallic);
    data.Specular = ComputeF0(0.5f, materialAlbedo, metallic);
    data.Roughness = roughness;
    return data;
}


#endif
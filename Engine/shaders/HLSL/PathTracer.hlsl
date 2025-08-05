#include "PathTracing.hlsli"

struct PathTracingConstants
{
    int bounceCount;
    int accumulatedFrames;
    uint accumIdx;
    uint outputIdx;
};
ConstantBuffer<PathTracingConstants> PathTracingPassCB : register(b1);

[shader("raygeneration")]
void PT_RayGen()
{
    RWTexture2D<float4> accumulationTexture = ResourceDescriptorHeap[PathTracingPassCB.accumIdx];

    float2 pixel = float2(DispatchRaysIndex().xy);
    float2 resolution = float2(DispatchRaysDimensions().xy);

    RNG rng = RNG_Initialize(pixel.x + pixel.y * resolution.x, frameData.frameCount, 16);
    float2 offset = float2(RNG_GetNext(rng), RNG_GetNext(rng));
    pixel += lerp(-0.5f.xx, 0.5f.xx, offset);

    float2 ncdXY = (pixel / (resolution * 0.5f)) - 1.0f;
    ncdXY.y *= -1.0f;
    float4x4 invViewProj = GetInverseViewProjectionMat();
    float4 rayStart = mul(float4(ncdXY, 1.0f, 1.0f), invViewProj);
    float4 rayEnd = mul(float4(ncdXY, 0.0f, 1.0f), invViewProj);

    rayStart.xyz /= rayStart.w;
    rayEnd.xyz /= rayEnd.w;
    float3 rayDir = normalize(rayEnd.xyz - rayStart.xyz);
    float rayLength = length(rayEnd.xyz - rayStart.xyz);

    RayDesc ray;
    ray.Origin = rayStart.xyz;
    ray.Direction = rayDir;
    ray.TMin = 0.0f;
    ray.TMax = FLT_MAX;

    float3 radiance = 0.0f;
    float3 throughput = 1.0f;
    float pdf = 1.0;
    for (int i = 0; i < PathTracingPassCB.bounceCount; ++i)
    {
        HitInfo info = (HitInfo) 0;
        if (TraceRay(ray, info))
        {
            Instance instanceData = GetInstanceData(info.instanceIndex);
            Mesh meshData = GetMeshData(instanceData.meshIndex);
            Material materialData = GetMaterialData(instanceData.materialIndex);
            VertexData vertex = LoadVertexData(meshData, info.primitiveIndex, info.barycentricCoordinates);

            float3 worldPosition = mul(vertex.pos, info.objectToWorldMatrix).xyz;
            float3 worldNormal = normalize(mul(vertex.nor, (float3x3) transpose(info.worldToObjectMatrix)));
            float3 geometryNormal = normalize(worldNormal);
            float3 V = -ray.Direction;
            MaterialProperties matProperties = GetMaterialProperties(materialData, vertex.uv, 0);
            BrdfData brdfData = GetBrdfData(matProperties);

            int lightIndex = 0;
            float lightWeight = 0.0f;

            float3 wo = normalize(GetCameraPosition() - worldPosition);
            if (SampleLightRIS(rng, worldPosition, worldNormal, lightIndex, lightWeight))
            {
                Light light = GetLightData(lightIndex);
                float visibility = TraceShadowRay(light, worldPosition.xyz);
                float3 wi = normalize(-light.direction.xyz);
                float NdotL = saturate(dot(worldNormal, wi));

                float3 directLighting = DefaultBRDF(wi, wo, worldNormal, brdfData.Diffuse, brdfData.Specular, brdfData.Roughness) * visibility * light.diffuseColor.rgb * NdotL;
                radiance += lightWeight * (directLighting + matProperties.emissive) * throughput / pdf;
            }

            if (i == PathTracingPassCB.bounceCount - 1)
                break;

            float probDiffuse = ProbabilityToSampleDiffuse(brdfData.Diffuse, brdfData.Specular);
            bool chooseDiffuse = RNG_GetNext(rng) < probDiffuse;

            float3 wi;
            if (chooseDiffuse)
            {
                wi = GetCosHemisphereSample(rng, worldNormal);

                float3 diffuseBrdf = DiffuseBRDF(brdfData.Diffuse);
                float NdotL = saturate(dot(worldNormal, wi));

                throughput *= diffuseBrdf * NdotL;
                pdf *= (NdotL / M_PI) * probDiffuse;
            }
            else
            {
                float2 u = float2(RNG_GetNext(rng), RNG_GetNext(rng));
                float3 H = SampleGGX(u, brdfData.Roughness, worldNormal);

                float roughness = max(brdfData.Roughness, 0.065);
                wi = reflect(-wo, H);

                float3 F;
                float3 specularBrdf = SpecularBRDF(worldNormal, wo, wi, brdfData.Specular, roughness, F);
                float NdotL = saturate(dot(worldNormal, wi));

                throughput *= specularBrdf * NdotL;

                float a = roughness * roughness;
                float D = D_GGX(worldNormal, H, a);
                float NdotH = saturate(dot(worldNormal, H));
                float LdotH = saturate(dot(wi, H));
                float NdotV = saturate(dot(worldNormal, wo));
                float samplePDF = D * NdotH / (4 * LdotH);
                pdf *= samplePDF * (1.0 - probDiffuse);
            }

            ray.Origin = OffsetRay(worldPosition, worldNormal);
            ray.Direction = wi;
            ray.TMin = 1e-2f;
            ray.TMax = FLT_MAX;
        }
        else
        {
            TextureCube envMapTexture = ResourceDescriptorHeap[frameData.envMapIdx];
            radiance += envMapTexture.SampleLevel(LinearWrapSampler, ray.Direction, 0).rgb * throughput / pdf;
            break;
        }
    }

    float3 previousColor = accumulationTexture[DispatchRaysIndex().xy].rgb;
    if (PathTracingPassCB.accumulatedFrames > 1)
    {
        radiance += previousColor;
    }

    if (any(isnan(radiance)) || any(isinf(radiance)))
    {
        radiance = float3(1, 0, 0);
    }
    
    accumulationTexture[DispatchRaysIndex().xy] = float4(radiance, 1.0);
    
    RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[PathTracingPassCB.outputIdx];
    outputTexture[DispatchRaysIndex().xy] = float4(radiance / PathTracingPassCB.accumulatedFrames, 1.0f);
}


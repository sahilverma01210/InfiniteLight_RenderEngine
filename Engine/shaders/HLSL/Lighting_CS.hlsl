#define BLOCK_SIZE 16

struct MaterialHandle
{
    int lightResourceHandlesIdx;
};

struct LightResourceHandles
{
    int renderTargetIdx;
    int frameConstIdx;
    int lightConstIdx;
    int shadowTexIdx;
    int fragPosTexIdx;
    int diffTexIdx;
    int normTexIdx;
    int specTexIdx;
};

struct FrameCBuffer
{
    float2 renderResolution;
};

struct PointLightProps
{
    float3 pos;
    float3 viewPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

struct CameraProps
{
    float4x4 cameraMat;
};

struct LightVector
{
    float3 vToL;
    float3 dirToL;
    float distToL;
};

SamplerComparisonState samplerCompareState : register(s0);
SamplerState samplerState : register(s1);

ConstantBuffer<CameraProps> cameraCB : register(b0);
ConstantBuffer<MaterialHandle> matCB : register(b1);

struct CSInput
{
    uint3 GroupId : SV_GroupID;
    uint3 GroupThreadId : SV_GroupThreadID;
    uint3 DispatchThreadId : SV_DispatchThreadID;
    uint GroupIndex : SV_GroupIndex;
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(CSInput input)
{
    ConstantBuffer<LightResourceHandles> lightResourceHandles = ResourceDescriptorHeap[matCB.lightResourceHandlesIdx];    
    
    ConstantBuffer<FrameCBuffer> frameCB = ResourceDescriptorHeap[lightResourceHandles.frameConstIdx];    
    ConstantBuffer<PointLightProps> pointLightCB = ResourceDescriptorHeap[lightResourceHandles.lightConstIdx];
    TextureCube smap = ResourceDescriptorHeap[lightResourceHandles.shadowTexIdx];
    Texture2D<float4> fragPosTex = ResourceDescriptorHeap[lightResourceHandles.fragPosTexIdx];
    Texture2D<float4> diffuseTex = ResourceDescriptorHeap[lightResourceHandles.diffTexIdx];
    Texture2D<float4> normalTex = ResourceDescriptorHeap[lightResourceHandles.normTexIdx];
    Texture2D<float4> specularTex = ResourceDescriptorHeap[lightResourceHandles.specTexIdx];
    
    RWTexture2D<float4> outTex = ResourceDescriptorHeap[lightResourceHandles.renderTargetIdx];
    
    float2 uv = ((float2) input.DispatchThreadId.xy + 0.5f) * 1.0f / frameCB.renderResolution;
    
    float4 position = fragPosTex.Sample(samplerState, uv);
    float4 diffuse = diffuseTex.Sample(samplerState, uv);
    float4 normal = normalTex.Sample(samplerState, uv);
    float4 specular = specularTex.Sample(samplerState, uv);
    
    float4 viewPos = float4((float3) mul(position, transpose(cameraCB.cameraMat)), 1.0f); // View space position
    
    matrix shadowViewProj =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -pointLightCB.pos.x, -pointLightCB.pos.y, -pointLightCB.pos.z, 1,
    };
    
    static const float zf = 100.0f;
    static const float zn = 0.5f;
    static const float c1 = zf / (zf - zn);
    static const float c0 = -zn * zf / (zf - zn);
    
    float4 shadowPosCS = mul(position, shadowViewProj); // Convert Shadow Position to Homogeneous Clip Space.
    
    // get magnitudes for each basis component
    const float3 m = abs(shadowPosCS).xyz;
    // get the length in the dominant axis
    // (this correlates with shadow map face and derives comparison depth)
    const float major = max(m.x, max(m.y, m.z));
    // converting from distance in shadow light space to projected depth
    float shadowDepth = (c1 * major + c0) / major;
    
    // shadow map test
    const float shadowLevel = smap.SampleCmpLevelZero(samplerCompareState, shadowPosCS.xyz, shadowDepth);
    
    // fragment to light vector data
    LightVector lv;
    lv.vToL = pointLightCB.viewPos - viewPos.xyz;
    lv.distToL = length(lv.vToL);
    lv.dirToL = lv.vToL / lv.distToL;
    
    // attenuation
    const float attenuation = 1.0f / (pointLightCB.attConst + pointLightCB.attLin * lv.distToL + pointLightCB.attQuad * (lv.distToL * lv.distToL));
    
    // diffuse light
    float3 diffuseLight = pointLightCB.diffuseColor * pointLightCB.diffuseIntensity * attenuation * max(0.0f, dot(lv.dirToL, normal.xyz)) * shadowLevel;
    
    // calculate reflected light vector
    const float3 w = normal.xyz * dot(lv.vToL, normal.xyz);
    const float3 r = normalize(w * 2.0f - lv.vToL);
    // vector from camera to fragment (in view space)
    const float3 viewCamToFrag = normalize(viewPos.xyz);
    // calculate specular component color based on angle between
    // viewing vector and reflection vector, narrow with power function
    float3 specularColor = pointLightCB.diffuseColor * pointLightCB.diffuseIntensity * specular.rgb;
    // specular reflected    
    float3 specularReflected = attenuation * specularColor * pow(max(0.0f, dot(-r, viewCamToFrag)), specular.a) * shadowLevel;

    // final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    outTex[input.DispatchThreadId.xy] = float4(saturate((diffuseLight + pointLightCB.ambient) * diffuse.rgb + specularReflected), 1.0f);
}
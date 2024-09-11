cbuffer LightCBuf : register(b1)
{
    float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer CubeColors : register(b2)
{
    float4 colors[1];
};

float4 PSMain() : SV_TARGET
{
    return colors[0];
}
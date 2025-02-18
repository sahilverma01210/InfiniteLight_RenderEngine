float3 materialColor : register(b0);

float4 main() : SV_Target
{
    return float4(materialColor, 1.0f);;
}
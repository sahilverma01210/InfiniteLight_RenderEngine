Texture2D tex;
SamplerState samp;

float4 PSMain(float2 uv : TEXCOORD) : SV_TARGET
{
    return tex.Sample(samp, uv);
}
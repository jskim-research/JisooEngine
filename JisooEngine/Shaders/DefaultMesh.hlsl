cbuffer DrawConstants : register(b0)
{
    row_major float4x4 WorldViewProjection;
    float4 BaseColor;
};

struct FVertexInput
{
    float3 Position : POSITION;
};

struct FVertexOutput
{
    float4 Position : SV_Position;
};

FVertexOutput VSMain(FVertexInput Input)
{
    FVertexOutput Output;
    Output.Position = mul(float4(Input.Position, 1.0f), WorldViewProjection);
    return Output;
}

float4 PSMain(FVertexOutput Input) : SV_Target0
{
    return BaseColor;
}

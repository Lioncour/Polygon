cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	float bias;
	float ratio;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	output.pos = pos;
	output.tex = float2((input.tex.x * ratio) + bias, input.tex.y);
	
	return output;
}
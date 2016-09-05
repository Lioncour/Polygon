// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : TEXCOORD0;
	float3 color : COLOR0;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float3 light = float3(-1.0f, -1.0f, -1.0f);
	light = normalize(light);
	
	float4 finalColor = float4(input.color, 1.0f);

	float value = dot(light, input.normal);
	
	//value = (value + 1) * 0.7f + 0.3f;
	//finalColor = saturate(value * float4(input.color, 1.0f));

	finalColor = float4(input.color, 1.0f) * 0.5f + saturate(value * float4(input.color, 1.0f));
	
	finalColor.a = 1;

	return finalColor;
}
